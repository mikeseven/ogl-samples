///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"
//#include "dsa.hpp"

namespace glu
{
	GLuint createTexture(char const* Filename)
	{
		gli::texture Texture(gli::load(Filename));
		if(Texture.empty())
			return 0;

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum Target = GL.translate(Texture.target());

		GLuint TextureName = 0;
		glGenTextures(1, &TextureName);
		glBindTexture(Target, TextureName);
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, Texture.levels() > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

		glm::tvec3<GLsizei> const Dimensions(Texture.extent());

		switch(Texture.target())
		{
		case gli::TARGET_1D:
			glTexStorage1D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal, Dimensions.x);
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			glTexStorage2D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal,
				Dimensions.x, Texture.target() == gli::TARGET_2D ? Dimensions.y : static_cast<GLsizei>(Texture.layers() * Texture.faces()));
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			glTexStorage3D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal,
				Dimensions.x, Dimensions.y, Texture.target() == gli::TARGET_3D ? Dimensions.z : static_cast<GLsizei>(Texture.layers() * Texture.faces()));
			break;
		default:
			assert(0);
			break;
		}

		for(std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
		for(std::size_t Face = 0; Face < Texture.faces(); ++Face)
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glm::tvec3<GLsizei> Dimensions(Texture.extent(Level));
			Target = gli::is_target_cube(Texture.target()) ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face) : Target;

			switch(Texture.target())
			{
			case gli::TARGET_1D:
				if(gli::is_compressed(Texture.format()))
					glCompressedTexSubImage1D(
						Target, static_cast<GLint>(Level), 0, Dimensions.x,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
				else
					glTexSubImage1D(
						Target, static_cast<GLint>(Level), 0, Dimensions.x,
						Format.External, Format.Type, Texture.data(Layer, Face, Level));
				break;
			case gli::TARGET_1D_ARRAY:
			case gli::TARGET_2D:
			case gli::TARGET_CUBE:
				if(gli::is_compressed(Texture.format()))
					glCompressedTexSubImage2D(
						Target, static_cast<GLint>(Level),
						0, 0, Dimensions.x, Texture.target() == gli::TARGET_1D_ARRAY ? static_cast<GLsizei>(Layer) : Dimensions.y,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
				else
					glTexSubImage2D(
						Target, static_cast<GLint>(Level),
						0, 0, Dimensions.x, Texture.target() == gli::TARGET_1D_ARRAY ? static_cast<GLsizei>(Layer) : Dimensions.y,
						Format.External, Format.Type, Texture.data(Layer, Face, Level));
				break;
			case gli::TARGET_2D_ARRAY:
			case gli::TARGET_3D:
			case gli::TARGET_CUBE_ARRAY:
				if(gli::is_compressed(Texture.format()))
					glCompressedTexSubImage3D(
						Target, static_cast<GLint>(Level),
						0, 0, 0, Dimensions.x, Dimensions.y, Texture.target() == gli::TARGET_3D ? Dimensions.z : static_cast<GLsizei>(Layer),
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
				else
					glTexSubImage3D(
						Target, static_cast<GLint>(Level),
						0, 0, 0, Dimensions.x, Dimensions.y, Texture.target() == gli::TARGET_3D ? Dimensions.z : static_cast<GLsizei>(Layer),
						Format.External, Format.Type, Texture.data(Layer, Face, Level));
				break;
			default: assert(0); break;
			}
		}
		return TextureName;
	}
}//namespace glu

namespace
{
	char const * VERT_SHADER_SOURCE("gl-450/direct-state-access.vert");
	char const * FRAG_SHADER_SOURCE("gl-450/direct-state-access.frag");
	char const * TEXTURE_DIFFUSE("kueken7_rgba8_srgb.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(160, 160);

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace program
	{
		enum type
		{
			VERTEX,
			FRAGMENT,
			MAX
		};
	}//namespace program

	namespace framebuffer
	{
		enum type
		{
			RENDER,
			RESOLVE,
			MAX
		};
	}//namespace framebuffer

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			TEXTURE,
			MULTISAMPLE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture
}//namespace

GLuint CreateFramebuffer(GLsizei layers, GLsizei width, GLsizei height, GLsizei samples, GLboolean fixedsamplelocations)
{
	GLuint framebuffer = 0;
	glCreateFramebuffers(1, &framebuffer);
	glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_LAYERS, layers);
	glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, width);
	glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, height);
	glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_SAMPLES, samples);
	glNamedFramebufferParameteri(framebuffer, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, fixedsamplelocations);
	return framebuffer;
}

void FramebufferAttachment(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
	glNamedFramebufferTexture(framebuffer, attachment, texture, level);
}

GLuint glCreateTextureGTC(GLenum target, GLsizei layers, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei samples, GLboolean fixedsamplelocations)
{
	GLuint texture = 0;
	glCreateTextures(target, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	switch (target)
	{
	case GL_TEXTURE_2D:
		assert(layers == 1 && width >= 1 && height >= 1 && depth == 1 && samples == 1 && fixedsamplelocations == GL_TRUE);
		glTextureStorage2D(texture, levels, internalformat, width, height);
		break;
	case GL_TEXTURE_2D_ARRAY:
		assert(layers >= 1 && width >= 1 && height >= 1 && depth == 1 && samples == 1 && fixedsamplelocations == GL_TRUE);
		glTextureStorage3D(texture, levels, internalformat, width, height, layers);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		assert(layers == 1 && width >= 1 && height >= 1 && depth == 1 && samples >= 1);
		glTextureStorage2DMultisample(texture, samples, internalformat, width, height, fixedsamplelocations);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		assert(layers >= 1 && width >= 1 && height >= 1 && depth == 1 && samples >= 1);
		glTextureStorage3DMultisample(texture, samples, internalformat, width, height, layers, fixedsamplelocations);
		break;
	case GL_TEXTURE_3D:
		assert(layers == 1 && width >= 1 && height >= 1 && depth >= 1 && samples == 1 && fixedsamplelocations == GL_TRUE);
		glTextureStorage3D(texture, levels, internalformat, width, height, depth);
		break;
	case GL_TEXTURE_CUBE_MAP:
		assert(layers == 1 && width >= 1 && height >= 1 && depth == 1 && samples == 1 && fixedsamplelocations == GL_TRUE);
		glTextureStorage2D(texture, levels, internalformat, width, height);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		assert(layers >= 1 && width >= 1 && height >= 1 && depth == 1 && samples == 1 && fixedsamplelocations == GL_TRUE);
		glTextureStorage3D(texture, levels, internalformat, width, height, layers * 6);
		break;
	}

	return texture;
}

void glTextureLevelsGTC(GLuint texture, GLint baseLevel, GLint maxLevel)
{
	glTextureParameteri(texture, GL_TEXTURE_BASE_LEVEL, baseLevel);
	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

void glTextureSwizzleGTC(GLuint texture, GLint red, GLint green, GLint blue, GLint alpha)
{
	glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_R, red);
	glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_G, green);
	glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_B, blue);
	glTextureParameteri(texture, GL_TEXTURE_SWIZZLE_A, alpha);
}

GLuint glCreateSamplerGTC(GLenum mag, GLenum min, GLenum mip, GLenum wrap, GLfloat borderColor[4], GLenum compare)
{
	assert(mag == GL_LINEAR || mag == GL_NEAREST);
	assert(min == GL_LINEAR || min == GL_NEAREST);
	assert(mip == GL_LINEAR || mip == GL_NEAREST);

	GLuint sampler = 0;
	glCreateSamplers(1, &sampler);

	GLint minFilter = GL_NONE;
	if (min == GL_LINEAR)
		minFilter = mip == GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
	else
		minFilter = mip == GL_LINEAR ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;

	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, mag);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, wrap);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, wrap);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, wrap);
	glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, borderColor);
	glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, compare == GL_NONE ? GL_NONE : GL_COMPARE_R_TO_TEXTURE);
	glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, compare != GL_NONE ? compare : GL_LEQUAL);

	return sampler;
}

GLuint glCreateVertexFormatGTC()
{
	GLuint vertexFormat = 0;
	glCreateVertexArrays(1, &vertexFormat);
	return vertexFormat;
}

void glVertexFormatAttribGTC(GLuint vertexFormat, GLuint attribindex, GLuint bindingindex, GLint size, GLenum internalType, GLenum externalType, GLboolean normalized, GLuint relativeoffset)
{
	assert((internalType == GL_INT && normalized == GL_FALSE) || internalType == GL_FLOAT || (internalType == GL_DOUBLE && normalized == GL_FALSE));
	glEnableVertexArrayAttrib(vertexFormat, attribindex);
	glVertexArrayAttribBinding(vertexFormat, attribindex, bindingindex);
	if (internalType == GL_INT)
		glVertexArrayAttribIFormat(vertexFormat, attribindex, size, externalType, relativeoffset);
	else if (internalType == GL_DOUBLE)
		glVertexArrayAttribLFormat(vertexFormat, attribindex, size, externalType, relativeoffset);
	else
		glVertexArrayAttribFormat(vertexFormat, attribindex, size, externalType, normalized, relativeoffset);
}

class instance : public test
{
public:
	instance(int argc, char* argv[]) :
		test(argc, argv, "gl-450-direct-state-access", test::CORE, 4, 5, glm::uvec2(640, 480), glm::vec2(glm::pi<float>() * 0.2f)),
		VertexArrayName(0),
		PipelineName(0),
		ProgramName(0),
		SamplerName(0),
		UniformBlockSize(0),
		UniformPointer(nullptr)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, texture::MAX> TextureName;
	std::array<GLuint, framebuffer::MAX> FramebufferName;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint SamplerName;
	GLint UniformBlockSize;
	glm::uint8* UniformPointer;

	bool initProgram()
	{
		bool Validated = true;

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 430 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 430 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.check_program(ProgramName);
		}

		if(Validated)
		{
			glCreateProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		this->UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glCreateBuffers(buffer::MAX, &BufferName[0]);
		glNamedBufferStorage(BufferName[buffer::ELEMENT], ElementSize, ElementData, 0);
		glNamedBufferStorage(BufferName[buffer::VERTEX], VertexSize, VertexData, 0);
		glNamedBufferStorage(BufferName[buffer::TRANSFORM], this->UniformBlockSize * 2, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		this->UniformPointer = static_cast<glm::uint8*>(glMapNamedBufferRange(
			BufferName[buffer::TRANSFORM], 0, this->UniformBlockSize * 2, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		return true;
	}

	bool initSampler()
	{
		glCreateSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		return true;
	}

	bool initTexture()
	{
		gli::texture2d Texture(gli::load(getDataDirectory() + TEXTURE_DIFFUSE));
		if(Texture.empty())
			return 0;

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum const Target = GL.translate(Texture.target());
		glm::tvec2<GLsizei> const Dimensions(Texture.extent());

		glCreateTextures(GL_TEXTURE_2D, 1, &TextureName[texture::TEXTURE]);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_MIN_FILTER, Texture.levels() > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
		glTextureParameteri(TextureName[texture::TEXTURE], GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);
		glTextureStorage2D(TextureName[texture::TEXTURE],
			static_cast<GLint>(Texture.levels()), Format.Internal,
			Dimensions.x, Texture.target() == gli::TARGET_2D ? Dimensions.y : static_cast<GLsizei>(Texture.layers() * Texture.faces()));

		for(gli::texture2d::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTextureSubImage2D(TextureName[texture::TEXTURE], static_cast<GLint>(Level),
				0, 0,
				static_cast<GLsizei>(Texture[Level].extent().x), static_cast<GLsizei>(Texture[Level].extent().y),
				Format.External, Format.Type,
				Texture[Level].data());
		}


		glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &TextureName[texture::MULTISAMPLE]);
		glTextureParameteri(TextureName[texture::MULTISAMPLE], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::MULTISAMPLE], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureStorage2DMultisample(TextureName[texture::MULTISAMPLE], 4, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_FALSE);

		glCreateTextures(GL_TEXTURE_2D, 1, &TextureName[texture::COLORBUFFER]);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage2D(TextureName[texture::COLORBUFFER], 1, GL_RGBA8, GLsizei(FRAMEBUFFER_SIZE.x), GLsizei(FRAMEBUFFER_SIZE.y));

		return true;
	}

	bool initFramebuffer()
	{
		glCreateFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glNamedFramebufferTexture(FramebufferName[framebuffer::RENDER], GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE], 0);
		glNamedFramebufferTexture(FramebufferName[framebuffer::RESOLVE], GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);

		if(glCheckNamedFramebufferStatus(FramebufferName[framebuffer::RENDER], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		if(glCheckNamedFramebufferStatus(FramebufferName[framebuffer::RESOLVE], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		GLint Samples = 0;
		//glGetNamedFramebufferParameteriv(FramebufferName[framebuffer::RENDER], GL_SAMPLES, &Samples);
//		if (Samples != 4)
//			return false;

		return true;
	}

	bool initVertexArray()
	{
		glCreateVertexArrays(1, &VertexArrayName);

		glVertexArrayAttribBinding(VertexArrayName, semantic::attr::POSITION, 0);
		glVertexArrayAttribFormat(VertexArrayName, semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(VertexArrayName, semantic::attr::POSITION);

		glVertexArrayAttribBinding(VertexArrayName, semantic::attr::TEXCOORD, 0);
		glVertexArrayAttribFormat(VertexArrayName, semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2));
		glEnableVertexArrayAttrib(VertexArrayName, semantic::attr::TEXCOORD);

		glVertexArrayElementBuffer(VertexArrayName, BufferName[buffer::ELEMENT]);
		glVertexArrayVertexBuffer(VertexArrayName, 0, BufferName[buffer::VERTEX], 0, sizeof(glf::vertex_v2fv2f));

		return true;
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();

		//glEnable(GL_SAMPLE_MASK);
		//glSampleMaski(0, 0xFF);

		return Validated;
	}

	bool end()
	{
		glUnmapNamedBuffer(BufferName[buffer::TRANSFORM]);

		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteSamplers(1, &SamplerName);

		return true;
	}

	void renderFBO()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(4.0f);

		glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		glViewportIndexedf(0, 0, 0, static_cast<float>(FRAMEBUFFER_SIZE.x), static_cast<float>(FRAMEBUFFER_SIZE.y));
		glClearNamedFramebufferfv(FramebufferName[framebuffer::RENDER], GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], 0, this->UniformBlockSize);
		//glBindSamplers(0, 1, &SamplerName);
		glBindTextureUnit(0, TextureName[texture::TEXTURE]);
		glBindVertexArray(VertexArrayName);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0, 0);

		glDisable(GL_MULTISAMPLE);
	}

	void renderFB()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearNamedFramebufferfv(0, GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], this->UniformBlockSize, this->UniformBlockSize);
		//glBindSamplers(0, 1, &SamplerName);
		glBindTextureUnit(0, TextureName[texture::COLORBUFFER]);
		glBindVertexArray(VertexArrayName);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0, 0);
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4 ProjectionA = glm::scale(glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f), glm::vec3(1, -1, 1));
			*reinterpret_cast<glm::mat4*>(this->UniformPointer + 0) = ProjectionA * this->view() * glm::mat4(1);

			glm::mat4 ProjectionB = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			*reinterpret_cast<glm::mat4*>(this->UniformPointer + this->UniformBlockSize) = ProjectionB * this->view() * glm::scale(glm::mat4(1), glm::vec3(2));
		}

		//glBindSampler(0, this->SamplerName);

		// Step 1, render the scene in a multisampled framebuffer
		glBindProgramPipeline(PipelineName);

		renderFBO();

		// Step 2: blit
		glBlitNamedFramebuffer(FramebufferName[framebuffer::RENDER], FramebufferName[framebuffer::RESOLVE],
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		GLenum MaxColorAttachment = GL_COLOR_ATTACHMENT0;
		glInvalidateNamedFramebufferData(FramebufferName[framebuffer::RENDER], 1, &MaxColorAttachment);

		// Step 3, render the colorbuffer from the multisampled framebuffer
		renderFB();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	instance Test(argc, argv);
	Error += Test();

	return Error;
}

