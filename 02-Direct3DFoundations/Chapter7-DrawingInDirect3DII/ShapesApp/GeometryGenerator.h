#pragma once
#include <stdint.h>
#include <DirectXMath.h>
#include <vector>
using uint16 = uint16_t;
using uint32 = uint32_t;

namespace Geometry
{
	struct Vertex
	{
		Vertex() = default;
		Vertex(
			const DirectX::XMFLOAT3& position,
			const DirectX::XMFLOAT3& normal,
			const DirectX::XMFLOAT3& tangent,
			const DirectX::XMFLOAT2& uv
			) : Position(position), Normal(normal), Tangent(tangent), UV(uv)
		{

		}

		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v
			) : Position(px,py,pz), Normal(nx,ny,nz), Tangent(tx,ty,tz), UV(u,v)
		{
			
		}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT2 UV;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices32;

		std::vector<uint16> & GetIndices16()
		{
			if (mIndices16.empty())
			{
				mIndices16.resize(Indices32.size());
				for (auto i = 0u; i < Indices32.size(); i++)
				{
					mIndices16[i] = static_cast<uint16>(Indices32[i]);
				}
			}

			return mIndices16;
		}

	private:
		std::vector<uint16> mIndices16;
	};

	class GeometryGenerator
	{
	public:
		GeometryGenerator();
		~GeometryGenerator();

		static MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);
		static void Subdivide(MeshData& meshData);
		static Vertex MidPoint(const Vertex& v0, const Vertex& v1);
		static MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);
		static MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);
		static MeshData CreateQuad(float x, float y, float w, float h, float depth);
		static MeshData CreateGeoSphere(float radius, uint32 numberOfSubdivisions);

	private:
		static void BuildCylinderTopCap(float topRadius, float height, uint32 sliceCount, MeshData& meshData);
		static void BuildCylinderBottomCap(float bottomRadius, float height, uint32 sliceCount, MeshData& meshData);
	};
}



