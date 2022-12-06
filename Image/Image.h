/*
Copyright 2022, Yi-Lin Wang
All Rights Reserved 
Permission to use, copy, modify, and distribute this software and 
its documentation for any non-commercial purpose is hereby granted 
without fee, provided that the above copyright notice appear in 
all copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the name of 
the author not be used in advertising or publicity pertaining to 
distribution of the software without specific, written prior 
permission. 
THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
ANY PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
*/

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


// 类成员函数的实现
template <class T>
Image<T>::Image()
{
	w = 0;
	h = 0;
	data = NULL;
	access = NULL;
}

template <class T>
Image<T>::Image(const int width, const int height, const bool init) 
{
	w = width;
	h = height;
	data = new T[w * h];  // allocate space for Image data, using 1d array
	access = new T*[h];   // allocate space for row pointers 

	// initialize row pointers
	for (int i = 0; i < h; i++)
		access[i] = data + (i * w);  // using 2d array to access 1d array element

	if (init)
		memset(data, 0, w * h * sizeof(T)); // initialize image with 0 value
}

template <class T>
void Image<T>::Resize(const int width, const int height, const bool init) 
{
	Release(); // before allocate space for new size, release memory first
	w = width;
	h = height;
	data = new T[w * h];  // allocate space for Image data
	access = new T*[h];   // allocate space for row pointers

	// initialize row pointers
	for (int i = 0; i < h; i++)
		access[i] = data + (i * w); // relate access with data

	if (init) 
		memset(data, 0, w * h * sizeof(T));
}


/**
 * @brief initialize image with specified value
 */
template <class T>
void Image<T>::Init(const T &val) 
{
	T *ptr = imPtr(this, 0, 0); // access 
	T *end = imPtr(this, w-1, h-1);
	while (ptr <= end)
		*ptr++ = val;
}

template <class T>
void Image<T>::Release()
{
	if(data)
		delete [] data;
	if(access)
		delete [] access;

	h = 0;
	w = 0;
}


template <class T>
Image<T> *Image<T>::Copy() const 
{
	Image<T> *im = new Image<T>(w, h, false);
	memcpy(im->data, data, w * h * sizeof(T));
	return im;
}
																									
template <class T>
Image<T>::~Image() 
{
	Release();
}


#endif