#include "stdafx.h"
#include "QuadTree.h"

#ifndef __VS2010
//** Rect **//
Rect_::Rect_(const Rect_ &other) : Rect_(other.x, other.y, other.width, other.height) { }
Rect_::Rect_(double _x, double _y, double _width, double _height) :
    x(_x),
    y(_y),
    width(_width),
    height(_height) {
}
bool Rect_::contains(const Rect_ &other) const noexcept {
	if (other.getLeft() < getLeft()) return false;
	if (other.getRight() > getRight()) return false;
	if (other.getTop() > getTop()) return false;
	if (other.getBottom() < getBottom()) return false;

    return true; // within bounds
}
bool Rect_::intersects(const Rect_ &other) const noexcept {
	if (other.getRight() < getLeft()) return false;
	if (other.getLeft() > getRight()) return false;
	if (other.getBottom() > getTop()) return false;
	if (other.getTop() < getBottom()) return false;

    return true; // intersection
}
double Rect_::getLeft()   const noexcept { return x - (width  * 0.5f); }
double Rect_::getTop()    const noexcept { return y + (height * 0.5f); }
double Rect_::getRight()  const noexcept { return x + (width  * 0.5f); }
double Rect_::getBottom() const noexcept { return y - (height * 0.5f); }

void Rect_::Add(Rect_ rect) {

	double fminx = min(x- width / 2, rect.x-  rect.width / 2.);
	double fmaxX = max(x + width / 2, rect.x + rect.width/2.);
	double fminy = min(y- height / 2, rect.y- rect.height / 2.);
	double fmaxy = max(y + height/2, rect.y + rect.height/2.);
	width = fmaxX - fminx;
	height = fmaxy - fminy;
	x = fminx + (width/2.);
	y = fminy + (height/2.);
	//x = min(x, rect.x);
	//y = min(y, rect.y);
	//width = max(x + width, rect.x + rect.width) - x;
	//height = max(y + height, rect.y + rect.height) - y;
}


//** Collidable **//
Collidable::Collidable(const Boundary &_bounds, void* _data) :
    bound(_bounds),
    data(_data) {
};

//** QuadTree **//
QuadTree::QuadTree() : QuadTree({}, 0, 0) { }
QuadTree::QuadTree(const QuadTree &other) : QuadTree(other.bounds, other.capacity, other.maxLevel) { }
QuadTree::QuadTree(const Boundary &_bound, unsigned _capacity, unsigned _maxLevel) :
    bounds(_bound),
    capacity(_capacity),
    maxLevel(_maxLevel) {
    objects.reserve(_capacity);
    foundObjects.reserve(_capacity);
}

// Inserts an object into this quadtree
bool QuadTree::insert(Collidable *obj) {
    if (obj->qt != nullptr) return false;

    if (!isLeaf) {
        // insert object into leaf
        if (QuadTree *child = getChild(obj->bound))
            return child->insert(obj);
    }
    objects.push_back(obj);
    obj->qt = this;

    // Subdivide if required
    if (isLeaf && level < maxLevel && objects.size() >= capacity) {
        subdivide();
        update(obj);
    }
    return true;
}

// Removes an object from this quadtree
bool QuadTree::remove(Collidable *obj) {
    if (obj->qt == nullptr) return false; // Cannot exist in vector
    if (obj->qt != this) return obj->qt->remove(obj);

    objects.erase(std::find(objects.begin(), objects.end(), obj));
    obj->qt = nullptr;
    discardEmptyBuckets();
    return true;
}

// Removes and re-inserts object into quadtree (for objects that move)
bool QuadTree::update(Collidable *obj) {
    if (!remove(obj)) return false;

    // Not contained in this node -- insert into parent
    if (parent != nullptr && !bounds.contains(obj->bound))
        return parent->insert(obj);
    if (!isLeaf) {
        // Still within current node -- insert into leaf
        if (QuadTree *child = getChild(obj->bound))
            return child->insert(obj);
    }
    return insert(obj);
}

// Searches quadtree for objects in the provided boundary and returns them in vector
std::vector<Collidable*> &QuadTree::getObjectsSameBound(const Boundary &bound) {
	foundSameObjects.clear();
	for (const auto &obj : objects) {
		// Only check for intersection with OTHER boundaries
		//if (&obj->bound != &bound && obj->bound.contains(bound))
		if (bound.contains(obj->bound))
			foundSameObjects.push_back(obj);
	}
	if (!isLeaf) {
		// Get objects from leaves
		if (QuadTree *child = getChild(bound)) {
			child->getObjectsSameBound(bound);
			foundSameObjects.insert(foundSameObjects.end(), child->foundSameObjects.begin(), child->foundSameObjects.end());
		}
		else for (QuadTree *leaf : children) {
			if (leaf->bounds.intersects(bound)) {
				leaf->getObjectsSameBound(bound);
				foundSameObjects.insert(foundSameObjects.end(), leaf->foundSameObjects.begin(), leaf->foundSameObjects.end());
			}
		}
	}
	return foundSameObjects;
}

// Searches quadtree for objects within the provided boundary and returns them in vector
std::vector<Collidable*> &QuadTree::getObjectsInBound(const Boundary &bound) {
    foundObjects.clear();
    for (const auto &obj : objects) {
        // Only check for intersection with OTHER boundaries
        if (&obj->bound != &bound && obj->bound.intersects(bound))
            foundObjects.push_back(obj);
    }
    if (!isLeaf) {
        // Get objects from leaves
        if (QuadTree *child = getChild(bound)) {
            child->getObjectsInBound(bound);
            foundObjects.insert(foundObjects.end(), child->foundObjects.begin(), child->foundObjects.end());
        } else for (QuadTree *leaf : children) {
            if (leaf->bounds.intersects(bound)) {
                leaf->getObjectsInBound(bound);
                foundObjects.insert(foundObjects.end(), leaf->foundObjects.begin(), leaf->foundObjects.end());
            }
        }
    }
    return foundObjects;
}



std::vector<Collidable*> &QuadTree::getObjectsIntersectBound(const Boundary &bound) {
	foundObjects.clear();
	for (const auto &obj : objects) {
		// Only check for intersection with OTHER boundaries
		if (!bound.contains(obj->bound) && obj->bound.intersects(bound))
			foundObjects.push_back(obj);
	}
	if (!isLeaf) {
		// Get objects from leaves
		if (QuadTree *child = getChild(bound)) {
			child->getObjectsIntersectBound(bound);
			foundObjects.insert(foundObjects.end(), child->foundObjects.begin(), child->foundObjects.end());
		}
		else for (QuadTree *leaf : children) {
			if (leaf->bounds.intersects(bound)) {
				leaf->getObjectsIntersectBound(bound);
				foundObjects.insert(foundObjects.end(), leaf->foundObjects.begin(), leaf->foundObjects.end());
			}
		}
	}
	return foundObjects;
}


// Returns total children count for this quadtree
unsigned QuadTree::totalChildren() const noexcept {
    unsigned total = 0;
    if (isLeaf) return total;
    for (QuadTree *child : children)
        total += child->totalChildren();
    return 4 + total;
}

// Returns total object count for this quadtree
unsigned QuadTree::totalObjects() const noexcept {
    unsigned total = (unsigned)objects.size();
    if (!isLeaf) {
        for (QuadTree *child : children)
            total += child->totalObjects();
    }
    return total;
}

// Removes all objects and children from this quadtree
void QuadTree::clear() noexcept {
    if (!objects.empty()) {
        for (auto&& obj : objects)
            obj->qt = nullptr;
        objects.clear();
    }
    if (!isLeaf) {
        for (QuadTree *child : children)
            child->clear();
        isLeaf = true;
    }
}

// Subdivides into four quadrants
void QuadTree::subdivide() {
    double width = bounds.width  * 0.5;
    double height = bounds.height * 0.5;
    double x = 0, y = 0;
	// 원점은 좌하단
    for (unsigned i = 0; i < 4; ++i) {
        switch (i) {
            case 0: x = bounds.x + (width / 2); y = bounds.y + (height / 2); break; // Top right
            case 1: x = bounds.x - (width / 2); y = bounds.y + (height / 2); break; // Top left
			case 2: x = bounds.x - (width / 2); y = bounds.y - (height / 2); break; // Bottom left
			case 3: x = bounds.x + (width / 2); y = bounds.y - (height / 2); break; // Bottom right
        }
        children[i] = new QuadTree({ x, y, width, height }, capacity, maxLevel);
        children[i]->level  = level + 1;
        children[i]->parent = this;
    }
    isLeaf = false;
}

// Discards buckets if all children are leaves and contain no objects
void QuadTree::discardEmptyBuckets() {
    if (!objects.empty()) return;
    if (!isLeaf) {
        for (QuadTree *child : children)
            if (!child->isLeaf || !child->objects.empty())
                return;
    }
    if (clear(), parent != nullptr)
        parent->discardEmptyBuckets();
}

// Returns child that contains the provided boundary
QuadTree *QuadTree::getChild(const Boundary &bound) const noexcept {
	double objLeft = bound.getLeft();
	double objRight = bound.getRight();
	double objTop = bound.getTop();
	double objBottom = bound.getBottom();

	bool left = (objLeft >= bounds.getLeft()) && (objRight <= bounds.x);
	bool right = (objLeft >= bounds.x) && (objRight <= bounds.getRight());
	bool top = (objTop <= bounds.getTop()) && (objBottom >= bounds.y);
	bool bottom = (objTop <= bounds.y) && (objBottom >= bounds.getBottom());

	if (top)
	{
		if (left)  return children[1]; // Top left
		if (right) return children[0]; // Top right
	}
	else if (bottom)
	{
		if (left)  return children[2]; // Bottom left
		if (right) return children[3]; // Bottom right
	}

    return nullptr; // Cannot contain boundary -- too large
}

QuadTree::~QuadTree() {
    clear();
    if (children[0]) delete children[0];
    if (children[1]) delete children[1];
    if (children[2]) delete children[2];
    if (children[3]) delete children[3];
}


#endif