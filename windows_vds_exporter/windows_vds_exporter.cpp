#include "stdafx.h"

_COM_SMARTPTR_TYPEDEF(IVdsServiceLoader, IID_IVdsServiceLoader);
_COM_SMARTPTR_TYPEDEF(IVdsService, IID_IVdsService);
_COM_SMARTPTR_TYPEDEF(IEnumVdsObject, IID_IEnumVdsObject);
_COM_SMARTPTR_TYPEDEF(IVdsProvider, IID_IVdsProvider);
_COM_SMARTPTR_TYPEDEF(IVdsSwProvider, IID_IVdsSwProvider);
_COM_SMARTPTR_TYPEDEF(IVdsPack, IID_IVdsPack);
_COM_SMARTPTR_TYPEDEF(IVdsVolume, IID_IVdsVolume);
_COM_SMARTPTR_TYPEDEF(IVdsVolumeMF, IID_IVdsVolumeMF);

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

	const char* pszMethod = getenv("HTTP_METHOD");
	bCgiMode = pszMethod != NULL;
	if (bCgiMode && strcmp(pszMethod, "GET") != 0)
	{
		printf("Status: 405 Method Not Allowed\n");
		exit(0);
	}

	if (bCgiMode)
	{
		printf("Content-Type: text/plain; charset=utf-8; version=0.0.4\n\n");
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
				CoTaskMemFree(pwszAccessPathArray[i]);
			CoTaskMemFree(pwszAccessPathArray);
			hr = pVdsVolumeMF->QueryReparsePoints(&pReparsePointArray, &lNumberOfResults);
			if (FAILED(hr))
			{
				fprintf(stderr, "IVdsVolumeMF::QueryReparsePoints() failed: hr=%08X\n", hr);
				exit(1);
			}
			for (LONG i = 0; i < lNumberOfResults; ++i)
				CoTaskMemFree(pReparsePointArray[i].pwszPath);
			CoTaskMemFree(pReparsePointArray);

			CoTaskMemFree(volume.pwszName);
		}
	}
}
