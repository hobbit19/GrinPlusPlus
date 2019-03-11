#include "WalletRestServer.h"
#include "WalletContext.h"
#include "../civetweb/include/civetweb.h"
#include "../RestUtil.h"
#include "API/OwnerAPI.h"

#include <Wallet/SessionTokenException.h>

WalletRestServer::WalletRestServer(const Config& config, IWalletManager& walletManager, INodeClient& nodeClient)
	: m_config(config), m_walletManager(walletManager)
{
	m_pWalletContext = new WalletContext(&walletManager, &nodeClient);
}

WalletRestServer::~WalletRestServer()
{
	delete m_pWalletContext;
}

bool WalletRestServer::Initialize()
{
	const uint32_t ownerPort = m_config.GetWalletConfig().GetOwnerPort();
	const char* pOwnerOptions[] = {
		"num_threads", "1",
		"listening_ports", std::to_string(ownerPort).c_str(),
		NULL
	};

	m_pOwnerCivetContext = mg_start(NULL, 0, pOwnerOptions);
	mg_set_request_handler(m_pOwnerCivetContext, "/v1/wallet/owner/", WalletRestServer::OwnerAPIHandler, m_pWalletContext);

	return true;
}

int WalletRestServer::OwnerAPIHandler(mg_connection* pConnection, void* pWalletContext)
{
	WalletContext* pContext = (WalletContext*)pWalletContext;

	const std::string action = RestUtil::GetURIParam(pConnection, "/v1/wallet/owner/");
	const EHTTPMethod method = RestUtil::GetHTTPMethod(pConnection);

	try
	{
		if (method == EHTTPMethod::GET)
		{
			return OwnerAPI::HandleGET(pConnection, action, *pContext->m_pWalletManager, *pContext->m_pNodeClient);
		}
		else if (method == EHTTPMethod::POST)
		{
			return OwnerAPI::HandlePOST(pConnection, action, *pContext->m_pWalletManager, *pContext->m_pNodeClient);
		}
	}
	catch (const SessionTokenException&)
	{
		return RestUtil::BuildUnauthorizedResponse(pConnection, "session_token is missing or invalid.");
	}
	catch (const DeserializationException&)
	{
		return RestUtil::BuildBadRequestResponse(pConnection, "Failed to deserialize one or more fields.");
	}
	catch (const std::exception&)
	{
		return RestUtil::BuildInternalErrorResponse(pConnection, "Unknown error occurred.");
	}

	return RestUtil::BuildBadRequestResponse(pConnection, "HTTPMethod not Supported");
}

bool WalletRestServer::Shutdown()
{
	mg_stop(m_pOwnerCivetContext);

	return true;
}