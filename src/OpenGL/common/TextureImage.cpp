#include "TextureImage.hpp"

namespace gl
{
	TextureImage::TextureImage(egl::Texture *parentTexture, GLsizei width, GLsizei height, GLenum format, GLenum type)
		: Image(parentTexture, width, height, format, type)
	{
	}

	TextureImage::TextureImage(egl::Texture *parentTexture, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type)
		: Image(parentTexture, width, height, depth, format, type)
	{
	}

	TextureImage::TextureImage(GLsizei width, GLsizei height, sw::Format internalFormat, int multiSampleDepth, bool lockable, bool renderTarget)
		: Image(width, height, internalFormat, multiSampleDepth, lockable, renderTarget)
	{
	}

	TextureImage::~TextureImage()
	{
	}

	void TextureImage::loadImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const UnpackInfo& unpackInfo, const void *input)
	{
		Image::loadImageData(xoffset, yoffset, zoffset, width, height, depth, format, type, unpackInfo, input);
	}

	void TextureImage::loadCompressedData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void *pixels)
	{
		Image::loadCompressedData(xoffset, yoffset, zoffset, width, height, depth, imageSize, pixels);
	}
}
