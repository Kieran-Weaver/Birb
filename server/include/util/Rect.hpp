#ifndef RECT_H
#define RECT_H
#include <algorithm>
#include <cstdint>
#include <vector>
template<typename T>
struct Rect{
	Rect() : left(0), top(0), width(0), height(0){
		static_assert(std::is_arithmetic<T>::value, "Integral required.");
	}

	Rect(const Rect<T>& r) : left(r.left), top(r.top), width(r.width), height(r.height) {
		static_assert(std::is_arithmetic<T>::value, "Integral required.");
	}

	Rect(T l, T t, T w, T h) : left(l), top(t), width(w), height(h) {
		static_assert(std::is_arithmetic<T>::value, "Integral required.");
	}

	Rect<T>& operator=(const Rect<T>& r){
		left = r.left;
		top = r.top;
		width = r.width;
		height = r.height;
		return *this;
	}
	
	T left;
	T top;
	T width;
	T height;

	bool Contains(float x, float y) const{
		return (((left - x)*(left + width - x) <= 0.f)&&((top - y)*(top + height - y) <= 0.f));
	}
	bool Contains(const Rect<T>& r) const{
		return (((r.left + r.width)<=(left+width))&&((r.top+r.height)<=(top+height))&&(r.left >= left) && (r.top >= top));
	}

	bool Intersects(const Rect<T>& r) const{
		T iLeft = std::max(left, r.left);
		T iTop = std::max(top, r.top);
		T iRight = std::min(left + width,r.left + r.width);
		T iBottom = std::min(top + height, r.top + r.height);
		return ((iLeft < iRight) && (iTop < iBottom));
	}

	bool RIntersects(const Rect<T>& r, Rect<T>& intersection) const{
		T iLeft = std::max(left, r.left);
		T iTop = std::max(top, r.top);
		T iRight = std::min(left + width,r.left + r.width);
		T iBottom = std::min(top + height, r.top + r.height);
		if ((iLeft < iRight) && (iTop < iBottom)){
			intersection = Rect<T>(iLeft,iTop,iRight - iLeft, iBottom-iTop);
			return true;
		}else{
			return false;
		}
	}

	Rect<T> getAABB() const{
		return *this;
	}
};
inline Rect<float> Normalize(const Rect<uint16_t>& texrect){
	return Rect<float>(texrect.left/65536.f,texrect.top/65536.f,texrect.width/65536.f,texrect.height/65536.f);
}
template<class T, class Iter>
Rect<T> join(Iter i, Iter last){
	T minX = std::numeric_limits<T>::max(), maxX = std::numeric_limits<T>::min();
	T minY = std::numeric_limits<T>::max(), maxY = std::numeric_limits<T>::min();
	while (i != last){
		minX = std::min(i->left, minX);
		maxX = std::max(i->left + i->width, maxX);
		minY = std::min(i->top, minY);
		maxY = std::max(i->top + i->height, maxY);
		i++;
	}
	return {minX, minY, maxX - minX, maxY - minY};
}
template<class T>
Rect<T> join(const Rect<T>& a, const Rect<T>& b){
	T minX = std::min(a.left, b.left);
	T maxX = std::max(a.left + a.width, b.left + b.width);
	T minY = std::min(a.top, b.top);
	T maxY = std::max(a.top + a.height, b.top + b.height);
	return {minX, minY, maxX - minX, maxY - minY};
}

template<class T>
void shift(Rect<T>& old, int direction, T xvalue, T yvalue){
	switch (direction){
	case 0:
		old.top -= yvalue;
		break;
	case 1:
		old.left += xvalue;
		break;
	case 2:
		old.top += yvalue;
		break;
	case 3:
		old.left -= xvalue;
		break;
	}
}
#endif
