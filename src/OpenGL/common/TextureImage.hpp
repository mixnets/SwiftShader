#ifndef gl_TextureImage_hpp
#define gl_TextureImage_hpp

#include "common/Image.hpp"

namespace gl
{
	GLsizei ComputePitch(GLsizei width, GLenum format, GLenum type, GLint alignment);
	GLsizei ComputeCompressedSize(GLsizei width, GLsizei height, GLenum format);

	class TextureImage : public egl::Image
	{
	public:
		TextureImage(egl::Texture *parentTexture, GLsizei width, GLsizei height, GLenum format, GLenum type);
		TextureImage(egl::Texture *parentTexture, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type);

		~TextureImage() override;

		void loadImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const UnpackInfo& unpackInfo, const void *input) override;
		void loadCompressedData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void *pixels) override;

	private:
		void loadD24S8ImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, int inputPitch, int inputHeight, const void *input, void *buffer);
		void loadD32FS8ImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, int inputPitch, int inputHeight, const void *input, void *buffer);
	};
}

#endif   // gl_TextureImage_hpp
