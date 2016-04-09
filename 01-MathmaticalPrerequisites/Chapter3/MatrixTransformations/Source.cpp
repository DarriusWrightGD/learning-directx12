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
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ") " << endl;

	return os;
}

ostream& XM_CALLCONV operator<<(ostream& os, FXMMATRIX m)
{
	for (int i = 0; i < 4; i++)
	{
		os << "\n" << XMVectorGetX(m.r[i]) << "\t";
		os << XMVectorGetY(m.r[i]) << "\t";
		os << XMVectorGetZ(m.r[i]) << "\t";
		os << XMVectorGetW(m.r[i]) << endl;
	}

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

	
	XMVECTOR a = XMVectorSet(2,1,3,1);

	XMMATRIX scale = XMMatrixScaling(2, .5, 1);

	XMMATRIX rotationX = XMMatrixRotationX(XMConvertToRadians(90.0f));
	XMMATRIX rotationY = XMMatrixRotationY(XMConvertToRadians(90.0f));
	XMMATRIX rotationZ = XMMatrixRotationZ(XMConvertToRadians(90.0f));

	XMMATRIX translate = XMMatrixTranslation(2, 3, 1);

	cout << "a: "<< a << endl;
	cout << "translation: " << translate << endl;
	cout << "scale : " << scale << endl;
	cout << "rotation x:" << rotationX << endl;
	cout << "rotation y:" << rotationY << endl;
	cout << "rotation z:" << rotationZ << endl;


	cout << "scale a: " << XMVector3Transform(a, scale) << endl;
	cout << "rotate x a: " << XMVector3Transform(a, rotationX) << endl;
	cout << "rotate y a: " << XMVector3Transform(a, rotationY) << endl;
	cout << "rotate z a: " << XMVector3Transform(a, rotationZ) << endl;
	cout << "translate a:" << XMVector3Transform(a, translate) << endl;


	system("pause");
	return 0;
}