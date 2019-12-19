#pragma once
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <directxmath.h>
using namespace DirectX;

class Camera
{
public:
	Camera();
	Camera(const Camera&);
	~Camera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition() {
		return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
	}

	XMFLOAT3 GetRotation() {
		return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
	}

	void Render();
	void GetViewMatrix(XMMATRIX& viewMatrix) {
		viewMatrix = m_viewMatrix;
	}

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	XMMATRIX m_viewMatrix;
};

#endif