#include "mkpch.h"

#include "model.h"

#include "MKEngine/Core/Log.h"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace MKEngine {

    void Model::Draw(const VkCommandBuffer commandBuffer) const
    {
        for (auto& mesh : m_meshes)
        {
            mesh.Draw(commandBuffer);
        }
    }

    Mesh ProcessMesh(aiMesh* aiMesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            vector.x = aiMesh->mVertices[i].x;
            vector.y = aiMesh->mVertices[i].y;
            vector.z = aiMesh->mVertices[i].z;
            vertex.Position = vector;

           /*
            Normals
             if (aiMesh->mNormals != nullptr) {
                vector.x = aiMesh->mNormals[i].x;
                vector.y = aiMesh->mNormals[i].y;
                vector.z = aiMesh->mNormals[i].z;

            }
            else {
                vector.x = 0;
                vector.y = 0;
                vector.z = 0;
            }
            vertex.normal = vector;
            
            */

            if (aiMesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = aiMesh->mTextureCoords[0][i].x;
                vec.y = aiMesh->mTextureCoords[0][i].y;
                vertex.TexCoord = vec;
            }
            else
                vertex.TexCoord = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
        {
	        const aiFace face = aiMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        Mesh mesh;
        mesh.Indices = indices;
        mesh.Vertices = vertices;
        mesh.Apply();
        return mesh;
    }

    Texture ProcessTexture(const aiTexture* aiTexture, const aiScene* scene)
    {
        TextureDescription description;
        description.Data = aiTexture->pcData;
        description.Width = aiTexture->mWidth;
        description.Height = aiTexture->mHeight;

        return  Texture::CreateTexture(description);
    }

    void ProcessNode(Model& model, const aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            auto processedMesh = ProcessMesh(mesh, scene);
            model.AddMesh(processedMesh);
        }

    	for (unsigned int i = 0; i < scene->mNumTextures; i++)
        {
	        const aiTexture* texture = scene->mTextures[i];
            auto processedTexture = ProcessTexture(texture, scene);
            model.AddTexture(processedTexture);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(model, node->mChildren[i], scene);
        }


    }

    Model Model::LoadModel(const char* filepath)
    {
    	Model model;

        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(filepath, aiProcess_Triangulate);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            MK_LOG_ERROR("Assimp error {}", import.GetErrorString());
        }

        ProcessNode(model, scene->mRootNode, scene);

        MK_LOG_INFO("Successfully loaded model: {0}", filepath);

		/*
		  DescriptorSetLayoutDescription descriptorSetLayoutDescription;

       

        DescriptorSetLayout::CreateDescriptorSetLayout(descriptorSetLayoutDescription);

	    for (auto texture : model.m_textures)
	    {
          
            descriptorSetLayoutDescription.AddBinding();
	    }
        descriptorSetLayoutDescription.AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
        descriptorSetLayoutDescription.AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

        m_descriptorSetLayout.CreateDescriptorSetLayout();
		 */

        return model;
    }

    void Model::DestroyModel(const Model& model)
    {
        for (auto mesh : model.m_meshes)
        {
            Mesh::Destroy(mesh);
        }
    }

    void Model::AddMesh(Mesh& mesh)
    {
        m_meshes.emplace_back(mesh);
    }

    void Model::AddTexture(Texture& texture)
    {
        m_textures.emplace_back(texture);
    }

   /*
     DescriptorSet Model::CreateDescriptorSet()
    {  //auto descriptorSet = DescriptorSet::Create(desc);
            //descriptorSet.BindBuffer(0, Buffers[i].UniformBuffer.Resource, 0, sizeof(UniformBufferObject));
            //descriptorSet.BindCombinedImageSampler(1, VulkanAPI::testTexture.View, VulkanAPI::testTexture.Sampler);
    }
    */
}
