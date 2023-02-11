#include "stdafx.h"

#include "EnumToString.h"

const char* VdsVolumeTypeToString(VDS_VOLUME_TYPE x)
{
	switch (x)
	{
	case VDS_VT_UNKNOWN:
		return "Unknown";
	case VDS_VT_SIMPLE:
		return "Simple";
	case VDS_VT_SPAN:
		return "Span";
	case VDS_VT_STRIPE:
		return "Stripe";
	case VDS_VT_MIRROR:
		return "Mirror";
	case VDS_VT_PARITY:
		return "Parity";
	default:
		return "Not Defined";
	}
}

const char* VdsVolumeStatusToString(VDS_VOLUME_STATUS x)
{
	switch (x)
	{
	case VDS_VS_UNKNOWN:
		return "Unknown";
	case VDS_VS_ONLINE:
		return "Online";
	case VDS_VS_NO_MEDIA:
		return "NoMedia";
	case VDS_VS_OFFLINE:
		return "Offline";
	case VDS_VS_FAILED:
		return "Failed";
	default:
		return "Not Defined";
	}
}

const char* VdsTransitionStateToString(VDS_TRANSITION_STATE x)
{
	switch (x)
	{
	case VDS_TS_UNKNOWN:
		return "Unknown";
	case VDS_TS_STABLE:
		return "Stable";
	case VDS_TS_EXTENDING:
		return "Extending";
	case VDS_TS_SHRINKING:
		return "Shrinking";
	case VDS_TS_RECONFIGING:
		return "Reconfiging";
	case VDS_TS_RESTRIPING:
		return "Restriping";
	default:
		return "Not Defined";
	}
}

const char* VdsHealthToString(VDS_HEALTH x)
{
	switch (x)
	{
	case VDS_H_UNKNOWN:
		return "Unknown";
	case VDS_H_HEALTHY:
		return "Healthy";
	case VDS_H_REBUILDING:
		return "Rebuilding";
	case VDS_H_STALE:
		return "Stale";
	case VDS_H_FAILING:
		return "Failing";
	case VDS_H_FAILING_REDUNDANCY:
		return "FailingRedundancy";
	case VDS_H_FAILED_REDUNDANCY:
		return "FailedRedundancy";
	case VDS_H_FAILED_REDUNDANCY_FAILING:
		return "FailedRedundancyFailing";
	case VDS_H_FAILED:
		return "Failed";
	case VDS_H_REPLACED:
		return "Replaced";
	case VDS_H_PENDING_FAILURE:
		return "PendingFailure";
	case VDS_H_DEGRADED:
		return "Degraded";
	default:
		return "Not Defined";
	}
}

std::string GuidToStdString(const GUID* x)
{
	RPC_CSTR buf;
	if (UuidToStringA(x, &buf) != RPC_S_OK)
		return std::string();
	std::string ret(reinterpret_cast<const char*>(buf));
	RpcStringFreeA(&buf);
	return ret;
}

std::string WideCharToUtf8StdString(const wchar_t* x)
{
	size_t len = wcslen(x);
	auto buflen = len * 3 + 1; // If wchar is not a surrogate, at most 3 bytes in UTF-8. If wchars are surrogate pair, at most 4 bytes (or 6 bytes in old spec) in UTF-8.
	char* buf = new char[buflen];
	if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, x, -1, buf, (int)buflen, NULL, NULL) == 0)
	{
		delete[] buf;
		return std::string();
	}
	std::string ret(buf);
	delete[] buf;
	return ret;
}
