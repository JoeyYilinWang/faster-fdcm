#ifndef _image_h_
#define _image_h_

#include <cstring>
#include <assert.h>
typedef unsigned char uchar;

/* use imRef to access Image data. */
#define imRef(im, x, y) (im->access[y][x])
  
/* use imPtr to get pointer to Image data. */
#define imPtr(im, x, y) &(im->access[y][x])

class RGBMap
{
public:
	RGBMap(uchar r,uchar g,uchar b): r_(r),g_(g),b_(b) {}; //有参数的构造函数
	RGBMap() {};
	uchar r_;
	uchar g_;
	uchar b_;
	inline RGBMap& operator=(const RGBMap &rhs); // 内联函数，对操作符“=”进行重载，返回值为引用
};


inline RGBMap& RGBMap::operator=(const RGBMap &rhs)
{
	r_ = rhs.r_;
	g_ = rhs.g_;
	b_ = rhs.b_;
	return (*this); // this表示该类对象的指针，即表示本身
}


template <class T> // 定义模板类
class Image 
{
	public:

	// constructor
	inline Image();

	/* create an Image */
	inline Image(const int width, const int height, const bool init = true);

	/* delete an Image */
	inline ~Image();

	/* release current image if any */
	inline void Release();

	inline void Resize(const int width,const int height, const bool init = true);


	/* init an Image */
	inline void Init(const T &val);

	/* copy an Image */
	inline Image<T> *Copy() const;

	/* get the width of an Image. */
	inline int width() const { return w; }

	/* get the height of an Image. */
	inline int height() const { return h; }
	
	// returning a reference to the parituclar location.
	inline T& Access(int x,int y) {return access[y][x];};


	/* Image data. */
	T *data;

	/* row pointers. */
	T **access;

private:
	int w, h;
};


#endif