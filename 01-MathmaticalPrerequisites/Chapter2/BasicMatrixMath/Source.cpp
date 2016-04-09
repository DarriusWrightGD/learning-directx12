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

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z<< ", " << dest.w  << ") " << endl;

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

	XMMATRIX A(
		1,0,2,1,
		4,3,5,3,
		1,2,1,0,
		1,2,3,2
	);

	XMMATRIX B = XMMatrixIdentity();
	XMVECTOR det;
	cout << "A: " << A << endl;
	cout << "B: " << B << endl;
	cout << "A*B= " << A*B << endl;
	cout << "transpose(A)= " << XMMatrixTranspose(A) << endl;
	cout << "determinate(A)= " << XMVectorGetX(XMMatrixDeterminant(A)) << endl;
	cout << "inverse(A)= " << XMMatrixInverse(&det,A) << endl;

	system("pause");
	return 0;
}