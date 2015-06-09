#include "Synergy/Graphics/Texture.hpp"

#include <Synergy/Core/Exception.hpp>
#include <Synergy/Graphics/GlErrorCheck.hpp>

namespace Synergy {

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
Texture<textureTarget>::Texture()
		: objName(0)
{

}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
Texture<textureTarget>::~Texture() {
	glDeleteTextures(1, &objName);
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
void Texture<textureTarget>::bind() const {
	glBindTexture(textureTarget, objName);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
void Texture<textureTarget>::unbind() const {
	glBindTexture(textureTarget, 0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
void Texture<textureTarget>::allocateStorage(
		const TextureSpec & spec,
		const GLvoid * data,
		GLint mipMaplevel
) {
	if (objName == 0) {
		glGenTextures(1, &objName);
		if (objName == 0) {
			throw Synergy::Exception(
					"Unable to generate texture name after calling glGenTextures(...)"
			);
		}
	}

	glBindTexture(textureTarget, objName);

	switch (type) {

		case GL_TEXTURE_3D:
			glTexImage3D(
					GL_TEXTURE_3D,
					mipMaplevel,
					spec.internalFormat,
					spec.width,
					spec.height,
					spec.depth,
					0, // border
					spec.format,
					spec.dataType,
					data
			);
	        break;

		case GL_TEXTURE_2D:
			glTexImage2D(
					GL_TEXTURE_2D,
					mipMaplevel,
					spec.internalFormat,
					spec.width,
					spec.height,
					0, // border
					spec.format,
					spec.dataType,
					data
			);
	        break;

		default:
			break;
	}

	setTextureParameters(spec);

	glBindTexture(textureTarget, 0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
void Texture<textureTarget>::setTextureParameters(
		const TextureSpec & spec
) {
	this->textureSpec = spec;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLuint Texture<textureTarget>::name() const {
	return objName;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLuint Texture<textureTarget>::width() const {
	return textureSpec.width;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLuint Texture<textureTarget>::height() const {
	return textureSpec.height;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLuint Texture<textureTarget>::depth() const {
	return textureSpec.depth;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLenum Texture<textureTarget>::internalFormat() const {
	return textureSpec.internalFormat;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLenum Texture<textureTarget>::format() const {
	return textureSpec.format;
}

//---------------------------------------------------------------------------------------
template<GLenum textureTarget>
GLenum Texture<textureTarget>::dataType() const {
	return textureSpec.dataType;
}


} // end namespace Synergy
