
#include "Precompiled.h"
#include "SoftRenderer.h"
#include <random>
using namespace CK::DD;

// 격자를 그리는 함수
void SoftRenderer::DrawGizmo2D()
{
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 그리드 색상
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// 뷰의 영역 계산
	Vector2 viewPos = g.GetMainCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// 좌측 하단에서부터 격자 그리기
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// 그리드가 시작되는 좌하단 좌표 값 계산
	Vector2 minPos = viewPos - extent;
	Vector2 minGridPos = Vector2(ceilf(minPos.X / (float)_Grid2DUnit), ceilf(minPos.Y / (float)_Grid2DUnit)) * (float)_Grid2DUnit;
	ScreenPoint gridBottomLeft = ScreenPoint::ToScreenCoordinate(_ScreenSize, minGridPos - viewPos);

	for (int ix = 0; ix < xGridCount; ++ix)
	{
		r.DrawFullVerticalLine(gridBottomLeft.X + ix * _Grid2DUnit, gridColor);
	}

	for (int iy = 0; iy < yGridCount; ++iy)
	{
		r.DrawFullHorizontalLine(gridBottomLeft.Y - iy * _Grid2DUnit, gridColor);
	}

	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	r.DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	r.DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}

// 게임 오브젝트 목록


// 최초 씬 로딩을 담당하는 함수
void SoftRenderer::LoadScene2D()
{
	// 최초 씬 로딩에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();

}

#pragma region Variables

// 게임 로직과 렌더링 로직이 공유하는 변수
Vector2 currentPosition;
float currentScale = 100.f;
float currentDegree = 0.f;

#pragma endregion 



// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();
	
	// 게임 로직의 로컬 변수
	static float moveSpeed = 100.f;
	static float scaleMin = 50.f;
	static float scaleMax = 200.f;
	static float scaleSpeed = 100.f;
	static float rotateSpeed = 180.f;

	Vector2 inputVector = Vector2(input.GetAxis(InputAxis::XAxis), input.GetAxis(InputAxis::YAxis)).GetNormalize();
	Vector2 deltaPosition = inputVector * moveSpeed * InDeltaSeconds;
	float deltaScale = input.GetAxis(InputAxis::ZAxis) * scaleSpeed * InDeltaSeconds;
	float deltaDegree = input.GetAxis(InputAxis::WAxis) * rotateSpeed * InDeltaSeconds;

	// 물체의 최종 상태 설정
	currentPosition += deltaPosition;
	currentScale = Math::Clamp(currentScale + deltaScale, scaleMin, scaleMax);
	currentDegree += deltaDegree;
}


static int circleFullfill_Time = 0;
// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();
	
	DrawGizmo2D();

	static constexpr float squareHalfSize = 0.5f;
	static constexpr size_t vertexCount = 4;
	static constexpr size_t triangleCount = 2;
	
	static constexpr std::array<Vertex2D, vertexCount> rawVertices = {
		Vertex2D(Vector2(-squareHalfSize, -squareHalfSize)),
		Vertex2D(Vector2(-squareHalfSize, squareHalfSize)),
		Vertex2D(Vector2(squareHalfSize, squareHalfSize)),
		Vertex2D(Vector2(squareHalfSize, -squareHalfSize)),
	};
	
	static constexpr std::array<size_t, triangleCount * 3> indices = {
		0, 1, 2,
		0, 2, 3
	};

	Vector3 sBasis1(currentScale, 0.f, 0.f);
	Vector3 sBasis2(0.f, currentScale, 0.f);
	Vector3 sBasis3 = Vector3::UnitZ;
	Matrix3x3 sMatrix(sBasis1, sBasis2, sBasis3);

	float sin = 0.f, cos = 0.f;
	Math::GetSinCos(sin, cos, currentDegree);
	Vector3 rBasis1(cos, sin, 0.f);
	Vector3 rBasis2(-sin, cos, 0.f);
	Vector3 rBasis3 = Vector3::UnitZ;
	Matrix3x3 rMatrix(rBasis1, rBasis2, rBasis3);

	Vector3 tBasis1 = Vector3::UnitX;
	Vector3 tBasis2 = Vector3::UnitY;
	Vector3 tBasis3(currentPosition.X, currentPosition.Y, 1.f);
	Matrix3x3 tMatrix(tBasis1, tBasis2, tBasis3);

	Matrix3x3 finalMatrix = tMatrix * rMatrix * sMatrix;

	// 행렬을 적용한 메시 정보를 사용해 물체를 렌더링
	static std::vector<Vertex2D> vertices(vertexCount);

	for (size_t vi = 0; vi < triangleCount; ++vi)
		vertices[vi].Position = finalMatrix * rawVertices[vi].Position;

	for (size_t ti = 0; ti < triangleCount; ++ti)
	{
		size_t bi = ti * 3;
		std::array<Vertex2D, 3> tv = { vertices[indices[bi]], vertices[indices[bi + 1]], vertices[indices[bi + 2]] };
		Vector2 minPos(Math::Min3(tv[0].Position.X, tv[1].Position.X, tv[2].Position.X), Math::Min3(tv[0].Position.Y, tv[1].Position.Y, tv[2].Position.Y));
		Vector2 maxPos(Math::Max3(tv[0].Position.X, tv[1].Position.X, tv[2].Position.X), Math::Max3(tv[0].Position.Y, tv[1].Position.Y, tv[2].Position.Y));

		// 무게중심좌표를 위한 준비작업
		Vector2 u = tv[1].Position - tv[0].Position;
		Vector2 v = tv[2].Position - tv[0].Position;

		float udotv = u.Dot(v);
		float vdotv = v.Dot(v);
		float udotu = u.Dot(u);

		// 분모
		float denominator = udotv * udotv - vdotv * udotu;

		if (denominator == 0.0f)
			continue;

		float invDenominator = 1.f / denominator;
		ScreenPoint lowerLeftPoint = ScreenPoint::ToScreenCoordinate(_ScreenSize, minPos);
		ScreenPoint upperRightPoint = ScreenPoint::ToScreenCoordinate(_ScreenSize, maxPos);

		lowerLeftPoint.X = Math::Max(0, lowerLeftPoint.X);
		lowerLeftPoint.Y = Math::Min(_ScreenSize.Y, lowerLeftPoint.Y);
		upperRightPoint.X = Math::Min(_ScreenSize.X, upperRightPoint.X);
		upperRightPoint.Y = Math::Max(0, upperRightPoint.Y);

		for (int x = lowerLeftPoint.X; x <= upperRightPoint.X; x++)
		{
			for (int y = upperRightPoint.Y; y <= lowerLeftPoint.Y; ++y)
			{
				ScreenPoint fragment = ScreenPoint(x, y);
				Vector2 pointToTest = fragment.ToCartesianCoordinate(_ScreenSize);
				Vector2 w = pointToTest - tv[0].Position;
				float wdotu = w.Dot(u);
				float wdotv = w.Dot(v);

				float s = (wdotv * udotv - wdotu * vdotv) * invDenominator;
				float t = (wdotu * udotv - wdotv * udotu) * invDenominator;
				float oneMinuST = 1.f - s - t;

				
				if (((s >= 0.f) && (s <= 1.f)) && ((t >= 0.f) && (t <= 1.f)) && ((oneMinuST >= 0.f) && (oneMinuST <= 1.f)))
				{
					r.DrawPoint(fragment, LinearColor::Blue);
				}

			}
		}

		r.DrawLine(vertices[indices[bi]].Position, vertices[indices[bi + 1]].Position, _WireframeColor);
		r.DrawLine(vertices[indices[bi]].Position, vertices[indices[bi + 2]].Position, _WireframeColor);
		r.DrawLine(vertices[indices[bi + 1]].Position, vertices[indices[bi + 2]].Position, _WireframeColor);
	}

	r.PushStatisticText(std::string("Position : ") + currentPosition.ToString());
	r.PushStatisticText(std::string("Scale : ") + std::to_string(currentScale));
	r.PushStatisticText(std::string("Degree : ") + std::to_string(currentDegree));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
