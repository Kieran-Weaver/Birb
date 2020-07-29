#ifndef RECTCOMPARE_H
#define RECTCOMPARE_H
#include <util/Rect.hpp>
#include <tuple>
template<typename T, bool D>
struct rectCompareImpl{
	static bool _(const Rect<T>& lhs, const Rect<T>& rhs);
};
template<typename T, bool D>
bool rectCompareImpl<T,D>::_(const Rect<T>& lhs, const Rect<T>& rhs){
	return std::tie(lhs.left, lhs.top) < std::tie(rhs.left, rhs.top);
}
template<typename T>
struct rectCompareImpl<T,true>{
	static bool _(const Rect<T>& lhs, const Rect<T>& rhs){
		return std::tie(lhs.top, lhs.left) < std::tie(rhs.top, rhs.left);
	}
};

template<typename T>
bool rectCompare(const Rect<T>& lhs, const Rect<T>& rhs, bool dim){
	if (dim){
		return rectCompareImpl<T, true>::_(lhs, rhs);
	} else {
		return rectCompareImpl<T, false>::_(lhs, rhs);
	}
}
#endif
