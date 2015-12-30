#ifndef gl_TextureImage_hpp
#define gl_TextureImage_hpp

#include "common/Image.hpp"

namespace gl
{
	class TextureImage : public egl::Image
	{
	public:
		TextureImage(egl::Texture *parentTexture, GLsizei width, GLsizei height, GLenum format, GLenum type);
		TextureImage(egl::Texture *parentTexture, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type);
		TextureImage(GLsizei width, GLsizei height, sw::Format internalFormat, int multiSampleDepth, bool lockable, bool renderTarget);

		~TextureImage() override;

		void loadImageData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const UnpackInfo& unpackInfo, const void *input) override;
		void loadCompressedData(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei imageSize, const void *pixels) override;
	};
}

#endif   // gl_TextureImage_hpp
