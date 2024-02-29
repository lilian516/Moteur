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
    
    m_pMousePose.x = 960;
    m_pMousePose.y = 540;

}

XMMATRIX Camera::update() {
    POINT currentCursorPose;
    GetCursorPos(&currentCursorPose);
    if (GetAsyncKeyState(VK_LBUTTON)) {
        if (m_pMousePose.y > currentCursorPose.y) {
            // Move camera up
            XMVECTOR upDirection = XMVector3Normalize(XMLoadFloat3(&m_fCameraUp));
            XMFLOAT3 upMovement;
            XMStoreFloat3(&upMovement, XMVectorScale(upDirection, m_fCameraMoveSpeed));
            m_fCameraPosition.x += upMovement.x;
            m_fCameraPosition.y += upMovement.y;
            m_fCameraPosition.z += upMovement.z;
            m_fCameraTarget.x += upMovement.x;
            m_fCameraTarget.y += upMovement.y;
            m_fCameraTarget.z += upMovement.z;
        }
        if (m_pMousePose.y < currentCursorPose.y) {
            // Move camera down
            XMVECTOR downDirection = XMVectorNegate(XMVector3Normalize(XMLoadFloat3(&m_fCameraUp)));
            XMFLOAT3 downMovement;
            XMStoreFloat3(&downMovement, XMVectorScale(downDirection, m_fCameraMoveSpeed));
            m_fCameraPosition.x += downMovement.x;
            m_fCameraPosition.y += downMovement.y;
            m_fCameraPosition.z += downMovement.z;
            m_fCameraTarget.x += downMovement.x;
            m_fCameraTarget.y += downMovement.y;
            m_fCameraTarget.z += downMovement.z;
        }
        if (m_pMousePose.x < currentCursorPose.x) {
            XMVECTOR left = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&m_fCameraUp), XMVectorSubtract(XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraPosition))));
            XMFLOAT3 leftMovement;
            XMStoreFloat3(&leftMovement, XMVectorScale(left, m_fCameraMoveSpeed));
            m_fCameraPosition.x += leftMovement.x;
            m_fCameraPosition.z += leftMovement.z;
            m_fCameraTarget.x += leftMovement.x;
            m_fCameraTarget.z += leftMovement.z;
        }
        if (m_pMousePose.x > currentCursorPose.x) {
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraPosition)), XMLoadFloat3(&m_fCameraUp)));
            XMFLOAT3 rightMovement;
            XMStoreFloat3(&rightMovement, XMVectorScale(right, m_fCameraMoveSpeed));
            m_fCameraPosition.x += rightMovement.x;
            m_fCameraPosition.z += rightMovement.z;
            m_fCameraTarget.x += rightMovement.x;
            m_fCameraTarget.z += rightMovement.z;
        }
    }
    
    
    
   
    
    return m_mViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_fCameraPosition), XMLoadFloat3(&m_fCameraTarget), XMLoadFloat3(&m_fCameraUp));
}
