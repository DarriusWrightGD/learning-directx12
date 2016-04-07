#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

using std::cout;
using std::endl;
using std::ostream;
using std::ios_base;

using namespace DirectX;
using namespace DirectX::PackedVector;

ostream& XM_CALLCONV operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ") " << endl;

	return os;
}

int main()
{
	cout.setf(ios_base::boolalpha);

	if (!XMVerifyCPUSupport())
	{
		cout << "DirectX Math is not supported on this machine" << endl;
		return -1;
	}

	XMVECTOR u = XMVectorSet(1.0f, 4.0f, 2.0f, 1.0f);
	XMVECTOR v = XMVectorSet(1.5f, 1.2f, 3.5f, 1.0f);
	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	float k = 2.5f;

	XMVECTOR proj;
	XMVECTOR perp;

	XMVector3ComponentsFromNormal(&proj, &perp, u, n);

	cout << "u: " << u << endl;
	cout << "v: " << v << endl;
	cout << "k: " << k << endl;
	cout << "u + v= " << u + v << endl;
	cout << "u - v= " << u - v << endl;
	cout << "ku= " << k * u << endl;
	cout << "u * v= " << XMVector3Dot(u, v) << endl;
	cout << "u x v= " << XMVector3Cross(u,v) << endl;
	cout << "||u||= " << XMVectorGetX(XMVector3Length(u)) << endl;
	cout << "u/||u||=" << XMVector3Normalize(u) << endl;
	cout << "projU: " << proj << endl;
	cout << "perpU: " << perp << endl;
	cout << "angle between projU and perpU: " << XMConvertToDegrees(XMVectorGetX(XMVector3AngleBetweenVectors(proj,perp))) << endl;

	system("pause");
	return 0;
}