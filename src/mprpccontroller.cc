#include "mprpccontroller.h"

MprpcController::MprpcController()
{
    Reset();
}

void MprpcController::Reset()
{
    _failed = false;
    _errText = "";
}

bool MprpcController::Failed() const 
{
    return _failed;
}
std::string MprpcController::ErrorText() const
{
    return _errText;
}

void MprpcController::SetFailed(const std::string& reason)
{
    _failed = true;
    _errText = reason;
}


// 目前未提供功能
void MprpcController::StartCancel() { }
bool MprpcController::IsCanceled() const { return false; }
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback) { }