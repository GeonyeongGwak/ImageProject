#pragma once
#include "PI_Blob.h" 
class QuadTree;
namespace PIAL
{
	//class PINSPALGO_API BlobRegion
	//{
	//public:
	//	BlobRegion();
	//	~BlobRegion();

	//	std::vector<PI_Blob_Res*> vecBlob;
	//};

	class PINSPALGO_API Dummy_Coli
	{
	public:
		double dummy1[4];
		void*  dummy2;
		void*  dummy3;
	};
	
	class PINSPALGO_API BlobBoundary
	{
	public:
		BlobBoundary();
		~BlobBoundary();

		void SetBoundary(int nWIdth, int nLength, int nCount, unsigned _capacity =3, unsigned _maxLevel=5);
		void Clear();
		void AddBlob(PI_Blob_Res blob);
		bool GetBlob_Bump(RECT rect, std::vector< PI_Blob_Res*>& blobs, int nType = -1); //Bump 검사 전용
		bool GetBlob_Bump_SafeThread(RECT rect, std::vector< PI_Blob_Res*>& blobs, int nType = -1); //Bump 검사 전용

		bool GetBlob(RECT rect,std::vector< PI_Blob_Res*>& blobs, int nType = -1);
		bool ContainBlob(RECT rect, std::vector< PI_Blob_Res*>& blobs, int nType = -1);
		bool HasBoundary();
		std::vector<PI_Blob_Res> vecBlob;

	private:
		QuadTree* m_qt;
		std::vector<Dummy_Coli> vecColidable;


	

	};
}

