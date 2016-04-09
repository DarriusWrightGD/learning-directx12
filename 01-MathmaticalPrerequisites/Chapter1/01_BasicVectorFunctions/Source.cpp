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

/**
	Passing XMVECTOR parameters
	1. The first 3 FXMVECTOR
	2. 4th GXMVECTOR
	3. 5th and 6th HXMVECTOR
	4. Afterward CXMVECTOR
**/

ostream& XM_CALLCONV operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ") " << endl;

	return os;
}

static const XMVECTORF32 ZERO_VECTOR = {0,0,0};

int main() 
{
	cout.setf(ios_base::boolalpha);

	if (!XMVerifyCPUSupport())
	{
		cout << "DirectX Math is not supported on this machine" << endl;
		return -1;
	}

	auto zeroVector = XMVectorZero();
	auto oneVector = XMVectorSplatOne();
	auto u = XMVectorSet(1,2,3,1);
	
	cout << XMVectorZero() << endl;
	cout << XMVectorReplicate(0.3f) << endl;
	cout << XMVectorSet(3.0f, 4.0f, 2.0f,1.0f) << endl;
	cout << XMVectorSplatY(u) << endl;

	system("pause");
	return 0;
}