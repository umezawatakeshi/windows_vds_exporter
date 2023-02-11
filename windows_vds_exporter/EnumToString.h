#pragma once

const char* VdsVolumeTypeToString(VDS_VOLUME_TYPE x);
const char* VdsVolumeStatusToString(VDS_VOLUME_STATUS x);
const char* VdsTransitionStateToString(VDS_TRANSITION_STATE x);
const char* VdsHealthToString(VDS_HEALTH x);
std::string GuidToStdString(const GUID* x);
std::string WideCharToUtf8StdString(const wchar_t* x);
