#include "Camera.h"

Camera::Camera() {

}

Camera::~Camera() {

}

void Camera::initCamera(int windowWidth, int windowHeight) {
    
    m_fCameraPosition = { 0.0f, 0.0f, -5.0f };
    m_fCameraTarget = { 0.0f, 0.0f, 0.0f };
    m_fCameraUp = { 0.0f, 1.0f, 0.0f };
    m_fCameraMoveSpeed = 0.1f;

    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    m_mProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
    m_mViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_fCameraPosition), XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraUp));
    //GetCursorPos(&m_pMousePose);
    //m_pMousePose = Point(400, 300);
    m_pMousePose.x = 400;
    m_pMousePose.y = 300;

}

XMMATRIX Camera::update() {
    POINT currentCursorPose;
    GetCursorPos(&currentCursorPose);
    if (m_pMousePose.x > currentCursorPose.x) {
        XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraPosition)));
        XMFLOAT3 forwardMovement;
        XMStoreFloat3(&forwardMovement, XMVectorScale(forward, m_fCameraMoveSpeed));
        m_fCameraPosition.x += forwardMovement.x;
        m_fCameraPosition.z += forwardMovement.z;
        m_fCameraTarget.x += forwardMovement.x;
        m_fCameraTarget.z += forwardMovement.z;
    }
    if (m_pMousePose.x < currentCursorPose.x) {
        XMVECTOR backward = XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&m_fCameraPosition), XMLoadFloat3(&m_fCameraTarget)));
        XMFLOAT3 backwardMovement;
        XMStoreFloat3(&backwardMovement, XMVectorScale(backward, m_fCameraMoveSpeed));
        m_fCameraPosition.x += backwardMovement.x;
        m_fCameraPosition.z += backwardMovement.z;
        m_fCameraTarget.x += backwardMovement.x;
        m_fCameraTarget.z += backwardMovement.z;
    }
    if (m_pMousePose.y > currentCursorPose.y) {
        XMVECTOR left = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&m_fCameraUp), XMVectorSubtract(XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraPosition))));
        XMFLOAT3 leftMovement;
        XMStoreFloat3(&leftMovement, XMVectorScale(left, m_fCameraMoveSpeed));
        m_fCameraPosition.x += leftMovement.x;
        m_fCameraPosition.z += leftMovement.z;
        m_fCameraTarget.x += leftMovement.x;
        m_fCameraTarget.z += leftMovement.z;
    }
    if (m_pMousePose.y < currentCursorPose.y) {
        XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraPosition)), XMLoadFloat3(&m_fCameraUp)));
        XMFLOAT3 rightMovement;
        XMStoreFloat3(&rightMovement, XMVectorScale(right, m_fCameraMoveSpeed));
        m_fCameraPosition.x += rightMovement.x;
        m_fCameraPosition.z += rightMovement.z;
        m_fCameraTarget.x += rightMovement.x;
        m_fCameraTarget.z += rightMovement.z;
    }
    
   
    
    return m_mViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_fCameraPosition), XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraUp));
}
