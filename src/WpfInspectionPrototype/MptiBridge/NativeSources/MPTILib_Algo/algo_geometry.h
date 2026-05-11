#pragma once
class algo_geometry
{
public:
	algo_geometry();
	~algo_geometry();


public:

	//Compute the dot product AB . BC
	static double DotProduct(POINTF pointA, POINTF pointB, POINTF pointC);

	//Compute the cross product AB x AC
	static double CrossProduct(POINTF pointA, POINTF pointB, POINTF pointC);

	//Compute the distance from A to B
	static double Distance(POINTF pointA, POINTF pointB);
	static double Distance(POINT pointA, POINT pointB);

	//Compute the distance from AB to C
    //if isSegment is true, AB is a segment, not a line.
	static double LineToPointDistance2D(POINTF pointA, POINTF pointB, POINTF pointC,bool isSegment);

	static double RectToPointDistance2D(RECT rect, POINT point, BOOL & bContain);

	// Point와 가장 가까운 Blob을 찾음
	static int RectCrossPoint(RECT* ptRect, int nNum, POINT point);
};

