#pragma once
//#include <any>
#include <vector>
#include <algorithm>

/*
 * 원점은 화면의 좌하단을 0,0으로 하고 우상단을 +로 잡음.
*/

//#define  __VS2010

#ifndef __VS2010


typedef struct Rect_ {
    double x, y, width, height;

    bool contains(const Rect_ &other) const noexcept;
    bool intersects(const Rect_ &other) const noexcept;
    double getLeft() const noexcept;
    double getTop() const noexcept;
    double getRight() const noexcept;
    double getBottom() const noexcept;

	void Add(Rect_ boundry);
    Rect_(const Rect_&);
    Rect_(double _x = 0, double _y = 0, double _width = 0, double _height = 0);
}Boundary;

struct Collidable {
    friend class QuadTree;
public:
	Boundary bound;
    void* data;

    Collidable(const Boundary &_bounds = {}, void* _data = {});
private:
    QuadTree *qt = nullptr;
    Collidable(const Collidable&) = delete;
};

class QuadTree {
public:
    QuadTree(const Boundary &_bound, unsigned _capacity, unsigned _maxLevel);
    QuadTree(const QuadTree&);
    QuadTree();

    bool insert(Collidable *obj);
    bool remove(Collidable *obj);
    bool update(Collidable *obj);
    std::vector<Collidable*> &getObjectsInBound(const Boundary &bound);
	std::vector<Collidable*> &getObjectsSameBound(const Boundary &bound);
	std::vector<Collidable*> &getObjectsIntersectBound(const Boundary &bound);

    unsigned totalChildren() const noexcept;
    unsigned totalObjects() const noexcept;
    void clear() noexcept;

    ~QuadTree();
private:
    bool      isLeaf = true;
    unsigned  level  = 0;
    unsigned  capacity;
    unsigned  maxLevel;
	Boundary      bounds;
    QuadTree* parent = nullptr;
    QuadTree* children[4] = { nullptr, nullptr, nullptr, nullptr };
    std::vector<Collidable*> objects, foundObjects, foundSameObjects;

    void subdivide();
    void discardEmptyBuckets();
    inline QuadTree *getChild(const Boundary &bound) const noexcept;
};

#endif // !__VS2010