#include "mkpch.h"
#include "camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace MKEngine {
	float Camera::GetNearClip() const
	{
		return m_near;
	}

	float Camera::GetFarClip() const
	{
		return m_far;
	}

	void Camera::SetPerspective(const float fov, const float aspect, const float znear, const float zfar)
	{
		this->m_fov = fov;
		this->m_near = znear;
		this->m_far = zfar;
		Matrices.Perspective = glm::perspectiveRH_ZO(glm::radians(fov), aspect, znear, zfar);
		if (FlipY) {
			Matrices.Perspective[1][1] *= -1.0f;
		}
	}

	void Camera::UpdateAspectRatio(const float aspect)
	{
		Matrices.Perspective = glm::perspective(glm::radians(m_fov), aspect, m_near, m_far);
		if (FlipY) {
			Matrices.Perspective[1][1] *= -1.0f;
		}
	}

	void Camera::SetPosition(const glm::vec3 position)
	{
		this->Position = position;
		m_isDirty = true;
	}

	void Camera::SetRotation(const glm::vec3 rotation)
	{
		this->Rotation = rotation;
		m_isDirty = true;
	}

	void Camera::Rotate(const glm::vec3 delta)
	{
		this->Rotation += delta;
		m_isDirty = true;
	}

	void Camera::SetTranslation(const glm::vec3 translation)
	{
		this->Position = translation;
		m_isDirty = true;
	}

	void Camera::Translate(const glm::vec3 delta)
	{
		this->Position += delta;
		m_isDirty = true;
	}

	void Camera::UpdateViewMatrix()
	{
		auto rotM = glm::mat4(1.0f);

		rotM = rotate(rotM, glm::radians(Rotation.x * (FlipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = rotate(rotM, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = rotate(rotM, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = Position;
		if (FlipY) {
			translation.y *= -1.0f;
		}
		const glm::mat4 transM = translate(glm::mat4(1.0f), translation);

		if (Type == FirstPerson)
		{
			Matrices.View = rotM * transM;
		}
		else
		{
			Matrices.View = transM * rotM;
		}

		ViewPos = glm::vec4(Position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
	}

	void Camera::Update(const float deltaTime, const CameraInput cameraInput)
	{
		if (cameraInput.moveInput.x != 0 || cameraInput.moveInput.y != 0)
		{
			m_isDirty = true;
		}
		if (cameraInput.moveInput.x != 0 || cameraInput.moveInput.y != 0)
		{
			if (Type == CameraType::FirstPerson)
			{
				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(Rotation.x)) * sin(glm::radians(Rotation.y));
				camFront.y = sin(glm::radians(Rotation.x));
				camFront.z = cos(glm::radians(Rotation.x)) * cos(glm::radians(Rotation.y));
				camFront = normalize(camFront);

				const float moveSpeed = deltaTime * 10;

				if (cameraInput.moveInput.y > 0)
					Position += camFront * moveSpeed;
				if (cameraInput.moveInput.y < 0)
					Position -= camFront * moveSpeed;
				if (cameraInput.moveInput.x < 0)
					Position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				if (cameraInput.moveInput.x > 0)
					Position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
			}
			else
			{
				Rotate(glm::vec3(0.0f, cameraInput.moveInput.x * 0.5f, 0.0f));
			}


			m_isDirty = true;
		}

		

		if(m_isDirty)
			UpdateViewMatrix();

	};

}