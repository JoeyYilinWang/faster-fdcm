#include "Image.h"

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
