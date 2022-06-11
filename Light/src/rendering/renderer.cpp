#include "light/rendering/renderer.hpp"
#include "light/rendering/rendercommand.hpp"

namespace Light
{
	Renderer::SceneData* Renderer::s_sceneData = new Renderer::SceneData;

	void Renderer::init()
	{
		RenderCommand::init();
	}

	void Renderer::onWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::setViewPort(0, 0, width, height);
	}

	void Renderer::beginScene(Camera& camera, glm::mat4 camera_view)
	{
		s_sceneData->viewProjectionMatrix = camera.getProjectionMatrix() * camera_view;

		glm::mat4 view = glm::mat4(glm::mat3(camera_view));
		s_sceneData->viewProjectionSkyboxMatrix = camera.getProjectionMatrix() * view;
		s_sceneData->cameraPosition = -glm::vec3(camera_view[3] * view);
	}

	void Renderer::beginScene(glm::mat4 viewProjectionMatrix, glm::vec3 position)
	{
		s_sceneData->viewProjectionMatrix = viewProjectionMatrix;

		// glm::mat4 view = glm::mat4(glm::mat3(camera_view));
		// s_sceneData->viewProjectionSkyboxMatrix = camera.getProjectionMatrix() * view;
		s_sceneData->cameraPosition = position;
		
	}

	void Renderer::endScene()
	{
	}

	void Renderer::submitLight(const std::vector<PointLight> &lights)
	{
		s_sceneData->pointLights = lights;
	}

	void Renderer::submitLight(const std::vector<SpotLight> &lights)
	{
		s_sceneData->spotLights = lights;
	}

	void Renderer::submitLight(const std::vector<DirectionalLight> &lights)
	{
		s_sceneData->directionalLights = lights;
	}

	void Renderer::submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vao, glm::mat4 transform)
	{
		submitID(shader, vao, transform);
	}

	void Renderer::submitID(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vao, glm::mat4 transform, int id, int depth_map_texture_unit)
	{
		vao->bind();

		shader->bind();

		shader->setUniformMat4("u_viewProjectionMatrix", s_sceneData->viewProjectionMatrix);
		shader->setUniformInt("depthMap", depth_map_texture_unit);

		for(DirectionalLight light : s_sceneData->directionalLights)
			shader->setUniformMat4("lightSpaceMatrix", light.getSpaceMatrix());

		// for(PointLight light : s_sceneData->pointLights)
		// 	shader->setUniformMat4("lightSpaceMatrix", light.getSpaceMatrix());

		shader->setUniformInt("u_id", id);

		shader->setUniformMat4("u_transform", transform);

		RenderCommand::drawIndexed(vao);

		shader->unbind();

		vao->unbind();
	}

	void Renderer::submitSkybox(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vao)
	{
		vao->bind();

		shader->bind();

		shader->setUniformMat4("u_viewProjectionMatrix", s_sceneData->viewProjectionSkyboxMatrix);
		shader->setUniformInt("u_cubemap", 0);

		RenderCommand::depthMask(false);

		RenderCommand::drawIndexed(vao);

		RenderCommand::depthMask(true);

		shader->unbind();

		vao->unbind();
	}

	void Renderer::submitForDirectionalShadow(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vao, glm::mat4 lightSpaceMatrix, glm::mat4 transform)
	{
		vao->bind();

		shader->bind();
		
		// (void)lightSpaceMatrix;
		shader->setUniformMat4("u_lightSpaceMatrix", lightSpaceMatrix);

		shader->setUniformMat4("u_transform", transform);

		RenderCommand::drawIndexed(vao);

		shader->unbind();

		vao->unbind();
	}
	void Renderer::submitForPointShadow(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vao,	std::vector<glm::mat4> lightSpaceMatrix, glm::mat4 transform)
	{
		vao->bind();

		shader->bind();
		
		for (unsigned int i = 0; i < 6; ++i)
            shader->setUniformMat4("shadowMatrices[" + std::to_string(i) + "]", lightSpaceMatrix[i]);

		shader->setUniformMat4("u_transform", transform);
		shader->setUniformFloat("far_plane", 25.0f);
		
		RenderCommand::drawIndexed(vao);

		shader->unbind();

		vao->unbind();
	}
}