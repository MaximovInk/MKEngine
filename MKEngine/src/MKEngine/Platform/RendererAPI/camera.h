#pragma once
#include <glm/glm.hpp>

namespace MKEngine
{
	struct CameraInput
	{
		glm::vec2 moveInput = glm::vec2();
		glm::vec2 mouseAxis = glm::vec2();
	};

	class Camera
	{
	public:
		enum CameraType { Lookat, FirstPerson };
		CameraType Type = CameraType::Lookat;

		glm::vec3 Rotation = glm::vec3();
		glm::vec3 Position = glm::vec3();
		glm::vec4 ViewPos = glm::vec4();

		bool FlipY = false;

		struct
		{
			glm::mat4 Perspective;
			glm::mat4 View;
		} Matrices;

		float GetNearClip() const;
		float GetFarClip() const;

		void SetPerspective(float fov, float aspect, float znear, float zfar);
		void UpdateAspectRatio(float aspect);
		void SetPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotation);
		void Rotate(glm::vec3 delta);
		void SetTranslation(glm::vec3 translation);
		void Translate(glm::vec3 delta);

		void Update(float deltaTime, CameraInput cameraInput= {});
	private:
		float m_fov{0};
		float m_near{ 0 }, m_far{ 0 };
		bool m_isDirty{ false };

		void UpdateViewMatrix();
	};

}
