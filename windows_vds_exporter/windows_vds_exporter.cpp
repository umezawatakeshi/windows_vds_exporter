#include "stdafx.h"

#include "EnumToString.h"

_COM_SMARTPTR_TYPEDEF(IVdsServiceLoader, IID_IVdsServiceLoader);
_COM_SMARTPTR_TYPEDEF(IVdsService, IID_IVdsService);
_COM_SMARTPTR_TYPEDEF(IEnumVdsObject, IID_IEnumVdsObject);
_COM_SMARTPTR_TYPEDEF(IVdsProvider, IID_IVdsProvider);
_COM_SMARTPTR_TYPEDEF(IVdsSwProvider, IID_IVdsSwProvider);
_COM_SMARTPTR_TYPEDEF(IVdsPack, IID_IVdsPack);
_COM_SMARTPTR_TYPEDEF(IVdsVolume, IID_IVdsVolume);
_COM_SMARTPTR_TYPEDEF(IVdsVolumeMF, IID_IVdsVolumeMF);

using labels_type = std::map<std::string, std::string>;

struct Gauge
{
	const char* const name;
	const char* const help;
	std::map<labels_type, double> values;

	void Serialize(FILE* fp) const;
};

void Gauge::Serialize(FILE* fp) const
{
	printf("\n");
	printf("# HELP %s %s\n", name, help);
	printf("# TYPE %s gauge\n", name);
	for (auto& [labels, value] : values)
	{
		printf("%s{", name);
		for (auto it = labels.begin(); it != labels.end(); ++it)
		{
			auto& [lkey, lval] = *it;
			if (it != labels.begin())
				putchar(',');
			printf("%s=\"", lkey.c_str());
			for (auto ch : lval)
			{
				switch (ch)
				{
				case '\\':
					printf("\\\\");
					break;
				case '\n':
					printf("\\n");
					break;
				case '"':
					printf("\\\"");
					break;
				default:
					putchar(ch);
				}
			}
			printf("\"");
		}
		printf("} %f\n", value);
	}
}

Gauge volume_info_gauge{ "windows_vds_volume_info","Volume information" };
Gauge volume_size_bytes_gauge{ "windows_vds_volume_size_bytes", "Volume size in bytes" };
Gauge volume_status_gauge{ "windows_vds_volume_status", "Volume status" };
Gauge volume_transition_state_gauge{ "windows_vds_volume_transition_state", "Volume transition state" };
Gauge volume_health_gauge{ "windows_vds_volume_health","Volume health" };
Gauge volume_access_path_gauge{ "windows_vds_volume_access_path","Volume access path" };
Gauge volume_reparse_point_gauge{ "windows_vds_volume_reparse_point","Volume reparse point" };

const Gauge* gauges[] = {
	&volume_info_gauge,
	&volume_size_bytes_gauge,
	&volume_status_gauge,
	&volume_transition_state_gauge,
	&volume_health_gauge,
	&volume_access_path_gauge,
	&volume_reparse_point_gauge,
};

bool bCgiMode = false;

void ProcessSoftwareProvider(IVdsProviderPtr& pVdsProvider, const VDS_PROVIDER_PROP& provider);

int main()
{
	HRESULT hr;
	IVdsServiceLoaderPtr pServiceLoader;
	IVdsServicePtr pService;
	IEnumVdsObjectPtr pEnumVdsProvider;
	IUnknownPtr pUnknown;
	ULONG cFetched;

	const char* pszMethod = getenv("REQUEST_METHOD");
	bCgiMode = pszMethod != NULL;
	if (bCgiMode && strcmp(pszMethod, "GET") != 0)
	{
		printf("Status: 405 Method Not Allowed\n");
		exit(0);
	}

	if (bCgiMode)
	{
		printf("Content-Type: text/plain; charset=utf-8; version=0.0.4\n\n");
		fflush(stdout);
	}
	_setmode(_fileno(stdout), _O_BINARY);

	printf(
		"# windows_vds_exporter, version 0.0.0\n"
		"# Copyright (c) 2023  UMEZAWA Takeshi\n"
		"# Licensed under GNU General Public License version 2 or later.\n"
	);

	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		fprintf(stderr, "CoInitialize() failed: hr=%08X\n", hr);
		exit(1);
	}

	hr = pServiceLoader.CreateInstance(CLSID_VdsLoader, NULL, CLSCTX_LOCAL_SERVER);
	if (FAILED(hr))
	{
		fprintf(stderr, "CoCreateInstance(CLSID_VdsLoader) failed: hr=%08X\n", hr);
		exit(1);
	}

	hr = pServiceLoader->LoadService(NULL, &pService);
	if (FAILED(hr))
	{
		fprintf(stderr, "IVdsServiceLoader::LoadService() failed: hr=%08X\n", hr);
		exit(1);
	}

	hr = pService->WaitForServiceReady();
	if (FAILED(hr))
	{
		fprintf(stderr, "IVdsService::WaitForServiceReady() failed: hr=%08X\n", hr);
		exit(1);
	}

	hr = pService->QueryProviders(VDS_QUERY_SOFTWARE_PROVIDERS, &pEnumVdsProvider);
	if (FAILED(hr))
	{
		fprintf(stderr, "IVdsService::QueryProviders() failed: hr=%08X\n", hr);
		exit(1);
	}
	while (pEnumVdsProvider->Next(1, &pUnknown, &cFetched) == S_OK)
	{
		IVdsProviderPtr pVdsProvider = pUnknown;
		pUnknown = NULL;

		VDS_PROVIDER_PROP provider;
		hr = pVdsProvider->GetProperties(&provider);
		if (FAILED(hr))
		{
			fprintf(stderr, "IVdsProvider::GetProperties() failed: hr=%08X\n", hr);
			exit(1);
		}
		switch (provider.type)
		{
		case VDS_PT_SOFTWARE:
			ProcessSoftwareProvider(pVdsProvider, provider);
			break;
		}
		CoTaskMemFree(provider.pwszName);
		CoTaskMemFree(provider.pwszVersion);
	}

	for (auto& gauge : gauges)
		gauge->Serialize(stdout);
}

void ProcessSoftwareProvider(IVdsProviderPtr& pVdsProvider, const VDS_PROVIDER_PROP& provider)
{
	HRESULT hr;
	IVdsSwProviderPtr pSwProvider = pVdsProvider;
	IEnumVdsObjectPtr pEnumVdsPack;
	IUnknownPtr pUnknown;
	ULONG cFetched;

	hr = pSwProvider->QueryPacks(&pEnumVdsPack);
	if (FAILED(hr))
	{
		fprintf(stderr, "IVdsSwProvider::QueryPacks() failed: hr=%08X\n", hr);
		exit(1);
	}
	while (pEnumVdsPack->Next(1, &pUnknown, &cFetched) == S_OK)
	{
		IVdsPackPtr pVdsPack = pUnknown;
		pUnknown = NULL;

		IEnumVdsObjectPtr pEnumVolume;

		hr = pVdsPack->QueryVolumes(&pEnumVolume);
		if (FAILED(hr))
		{
			fprintf(stderr, "IVdsPack::QueryVolumes() failed: hr=%08X\n", hr);
			exit(1);
		}
		while (pEnumVolume->Next(1, &pUnknown, &cFetched) == S_OK)
		{
			IVdsVolumePtr pVdsVolume = pUnknown;
			IVdsVolumeMFPtr pVdsVolumeMF = pUnknown;
			pUnknown = NULL;

			VDS_VOLUME_PROP volume;
			hr = pVdsVolume->GetProperties(&volume);
			if (FAILED(hr))
			{
				fprintf(stderr, "IVdsVolume::GetProperties() failed: hr=%08X\n", hr);
				exit(1);
			}
			std::string volume_id = GuidToStdString(&volume.id);
			volume_info_gauge.values.insert({ {
				{"volume_id", volume_id},
				{"name", WideCharToUtf8StdString(volume.pwszName)},
				{"type", VdsVolumeTypeToString(volume.type)},
			}, 1 });
			volume_size_bytes_gauge.values.insert({ {
				{"volume_id", volume_id},
			}, (double)volume.ullSize });
			volume_status_gauge.values.insert({ {
				{"volume_id", volume_id},
				{"status", VdsVolumeStatusToString(volume.status)},
			}, 1 });
			volume_transition_state_gauge.values.insert({ {
				{"volume_id", volume_id},
				{"transition_state", VdsTransitionStateToString(volume.TransitionState)},
			}, 1 });
			volume_health_gauge.values.insert({ {
				{"volume_id", volume_id},
				{"health", VdsHealthToString(volume.health)},
			}, 1 });

			LONG lNumberOfResults;
			LPWSTR* pwszAccessPathArray;
			VDS_REPARSE_POINT_PROP* pReparsePointArray;
			hr = pVdsVolumeMF->QueryAccessPaths(&pwszAccessPathArray, &lNumberOfResults);
			if (FAILED(hr))
			{
				fprintf(stderr, "IVdsVolumeMF::QueryAccessPaths() failed: hr=%08X\n", hr);
				exit(1);
			}
			for (LONG i = 0; i < lNumberOfResults; ++i)
			{
				volume_access_path_gauge.values.insert({ {
					{"volume_id", volume_id},
					{"access_path", WideCharToUtf8StdString(pwszAccessPathArray[i])},
				}, 1 });
				CoTaskMemFree(pwszAccessPathArray[i]);
			}
			CoTaskMemFree(pwszAccessPathArray);
			hr = pVdsVolumeMF->QueryReparsePoints(&pReparsePointArray, &lNumberOfResults);
			if (FAILED(hr))
			{
				fprintf(stderr, "IVdsVolumeMF::QueryReparsePoints() failed: hr=%08X\n", hr);
				exit(1);
			}
			for (LONG i = 0; i < lNumberOfResults; ++i)
			{
				volume_reparse_point_gauge.values.insert({ {
					{"volume_id", volume_id},
					{"source_volume_id", GuidToStdString(&pReparsePointArray[i].SourceVolumeId)},
					{"path", WideCharToUtf8StdString(pReparsePointArray[i].pwszPath)},
				}, 1 });
				CoTaskMemFree(pReparsePointArray[i].pwszPath);
			}
			CoTaskMemFree(pReparsePointArray);

			CoTaskMemFree(volume.pwszName);
		}
	}
}
