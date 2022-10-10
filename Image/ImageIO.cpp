#include "ImageIO.h"

/**
 * @brief load image from file and save it into Image<T> object
 */
template <class T>
void ImageIO::LoadImage(Image<T> **im, const char *name) 
{
	char buf[BUF_SIZE];

	/* read header */
	std::ifstream file(name, std::ios::in | std::ios::binary);
	pnm_read(file, buf);
	if (strncmp(buf, "VLIB", 9))
		throw pnm_error();

	pnm_read(file, buf);
	int width = atoi(buf);
	pnm_read(file, buf);
	int height = atoi(buf);

	/* read data */
	*im = new Image<T>(width, height);
	file.read((char *)imPtr((*im), 0, 0), width * height * sizeof(T));
}

/**
 * @brief save Image<T> object into a file
 */
template <class T>
void ImageIO::SaveImage(Image<T> *im, const char *name) 
{
	int width = im->width();
	int height = im->height();
	std::ofstream file(name, std::ios::out | std::ios::binary);

	file << "VLIB\n" << width << " " << height << "\n";
	file.write((char *)imPtr(im, 0, 0), width * height * sizeof(T));
}

void ImageIO::pnm_read(std::ifstream &file, char *buf) 
{
	char doc[BUF_SIZE];
	char c;

	file >> c; 
	while (c == '#') // 以#打头的行应该是注释行
	{
		file.getline(doc, BUF_SIZE); // 读取一行，将注释行读入到doc中，遇到换行符停止。注意，读入换行符但并不存储换行符
		file >> c; // 将实际内容存入到c中
	}
	file.putback(c); // 把c放回流缓冲区中
	file.width(BUF_SIZE); // 设置下一个输出的字段宽度。注意，只影响下一字段的字段宽度，之后将恢复默认值
	file >> buf; 
	file.ignore(); //为默认参数，.gnore(1, EOF)，其意义为将最后一个字符删除掉
}