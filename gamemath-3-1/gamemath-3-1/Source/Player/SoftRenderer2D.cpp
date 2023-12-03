
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

Vector2 point(0.f, 250.f);
Vector2 lineStart(-400.f, 0.f);
Vector2 lineEnd(400.f, 0.f);
float currentDegree = 0.f;
#pragma endregion 



// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();
	
	static float duration = 6.f;
	static float elapsedTime = 0.f;
	static float rotateSpeed = 180.f;
	static float distance = 250.f;
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> randomY(-200.f, 200.f);
	
	elapsedTime = Math::Clamp(elapsedTime + InDeltaSeconds, 0.f, duration);

	if (elapsedTime == duration)
	{
		lineStart = Vector2(-400.f, randomY(mt));
		lineEnd = Vector2(400.f, randomY(mt));
		elapsedTime = 0.f;
	}

	currentDegree = Math::FMod(currentDegree + rotateSpeed * InDeltaSeconds, 360.f);
	float sin = 0.f, cos = 0.f;
	Math::GetSinCos(sin, cos, currentDegree);
	point = Vector2(cos, sin) * distance;
}


static int circleFullfill_Time = 0;
// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();
	
	// 렌더링 로직 로컬 변수
	static std::vector<Vector2> circle;
	static float circleRadius = 5.f;

	if (circle.empty())
	{
		for (float x = -circleRadius; x <= circleRadius; ++x)
		{
			for (float y = -circleRadius; y <= circleRadius; ++y)
			{
				Vector2 target(x, y);
				float sizeSquared = target.SizeSquared();
				float rr = circleRadius * circleRadius;
				if (sizeSquared < rr)
				{
					circle.push_back(target);
				}
			}
		}
	}

	for (auto const& v : circle)
		r.DrawPoint(v + point, LinearColor::Red);

	r.DrawLine(lineStart, lineEnd, LinearColor::Black);
	r.DrawLine(lineStart, point, LinearColor::Red);

	Vector2 unitV = (lineEnd - lineStart).GetNormalize();
	Vector2 u = point - lineStart;
	Vector2 projV = unitV * (u.Dot(unitV));
	Vector2 projectedPoint = lineStart + projV;
	float distance = (projectedPoint - point).Size();

	for (auto const& v : circle)
		r.DrawPoint(v + projectedPoint, LinearColor::Magenta);

	// 투영 라인 그리기
	r.DrawLine(projectedPoint, point, LinearColor::Gray);
	r.PushStatisticText(std::string("Point : ") + point.ToString());
	r.PushStatisticText(std::string("Projected Point : ") + projectedPoint.ToString());
	r.PushStatisticText(std::string("Distance : ") + std::to_string(distance));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
