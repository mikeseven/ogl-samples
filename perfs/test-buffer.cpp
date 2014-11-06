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

namespace
{
	char const * VERT_SHADER_SOURCE("perfs/test-buffer.vert");
	char const * FRAG_SHADER_SOURCE("perfs/test-buffer.frag");

	struct vertex
	{
		vertex(){}

		vertex
		(
			glm::vec3 const & Position,
			glm::u8vec4 const & Color
		) :
			Position(Position),
			Color(Color)
		{}

		glm::vec3 Position;
		glm::u8vec4 Color;
	};

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
}//namespace

class test_buffer : public test
{
public:
	test_buffer(int argc, char* argv[], std::size_t FrameCount, glm::uvec2 const & WindowSize)
		: test(argc, argv, "test_buffer", test::CORE, 3, 3, FrameCount, RUN_ONLY, WindowSize)
		, VertexArrayName(0)
		, ProgramName(0)
		, ElementCount(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLsizei ElementCount;
	std::vector<vertex> VertexData;
	std::vector<glm::uint32> ElementData;

	glm::u8vec4 generateColorU8() const
	{
		return glm::u8vec4(glm::clamp(glm::linearRand(glm::vec4(0.0), glm::vec4(255.0)), glm::vec4(0.0), glm::vec4(255.0)));
	}

	glm::vec4 generateColor() const
	{
		return glm::clamp(glm::linearRand(glm::vec4(0.0), glm::vec4(255.0)), glm::vec4(0.0), glm::vec4(255.0));
	}

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE);

			ProgramName = glCreateProgram();
			glProgramParameteri(this->ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(this->ProgramName, VertShaderName);
			glAttachShader(this->ProgramName, FragShaderName);
			glLinkProgram(this->ProgramName);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(this->ProgramName);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &this->PipelineName);
			glUseProgramStages(this->PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, this->ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		glm::uvec2 const WindowSize(this->getWindowSize());

		VertexData.resize(WindowSize.x * WindowSize.y * 4);
		ElementData.resize(WindowSize.x * WindowSize.y * 6);
		this->ElementCount = static_cast<GLsizei>(ElementData.size());

		for(std::size_t j = 0; j < WindowSize.y >> 1; ++j)
		for(std::size_t i = 0; i < WindowSize.x >> 1; ++i)
		{
			std::size_t Index(i + j * (static_cast<std::size_t>(WindowSize.x) >> 1));
			VertexData[Index * 4 + 0] = vertex(glm::vec3(i * 2 + 0, j * 2 + 0, 0), this->generateColorU8());
			VertexData[Index * 4 + 1] = vertex(glm::vec3(i * 2 + 2, j * 2 + 0, 0), this->generateColorU8());
			VertexData[Index * 4 + 2] = vertex(glm::vec3(i * 2 + 2, j * 2 + 2, 0), this->generateColorU8());
			VertexData[Index * 4 + 3] = vertex(glm::vec3(i * 2 + 0, j * 2 + 2, 0), this->generateColorU8());
			ElementData[Index * 6 + 0] = static_cast<glm::uint32>(Index * 4 + 0);
			ElementData[Index * 6 + 1] = static_cast<glm::uint32>(Index * 4 + 1);
			ElementData[Index * 6 + 2] = static_cast<glm::uint32>(Index * 4 + 2);
			ElementData[Index * 6 + 3] = static_cast<glm::uint32>(Index * 4 + 2);
			ElementData[Index * 6 + 4] = static_cast<glm::uint32>(Index * 4 + 3);
			ElementData[Index * 6 + 5] = static_cast<glm::uint32>(Index * 4 + 0);
		}

		glm::mat4 Perspective = glm::ortho(0.0f, static_cast<float>(WindowSize.x), 0.0f, static_cast<float>(WindowSize.y));

		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementData.size() * sizeof(glm::uint32), &ElementData[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexData.size() * sizeof(vertex), &VertexData[0], GL_DYNAMIC_DRAW); //GL_STATIC_DRAW GL_STREAM_DRAW GL_DYNAMIC_DRAW
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Perspective), &Perspective[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), BUFFER_OFFSET(0));
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), BUFFER_OFFSET(sizeof(glm::vec3)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return true;
	}

	bool init()
	{
		bool Validated = true;
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initProgram();
		return Validated;
	}

	bool begin()
	{
		bool Validated = this->init();

		if(Validated)
		{
			glm::vec2 WindowSize(this->getWindowSize());
			glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

			glBindProgramPipeline(this->PipelineName);
			glBindVertexArray(this->VertexArrayName);
			glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, this->BufferName[buffer::TRANSFORM]);
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

			//glEnable(GL_DEPTH_TEST);
			//glDepthFunc(GL_LESS);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		}

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &this->BufferName[0]);
		glDeleteProgramPipelines(1, &this->PipelineName);
		glDeleteProgram(this->ProgramName);
		glDeleteVertexArrays(1, &this->VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::uvec2 const WindowSize(this->getWindowSize());
/*
		for(std::size_t j = 0; j < WindowSize.y >> 1; ++j)
		for(std::size_t i = 0; i < WindowSize.x >> 1; ++i)
		{
			std::size_t Index(i + j * (static_cast<std::size_t>(WindowSize.x) >> 1));
			VertexData[Index * 4 + 0] = vertex(glm::vec3(i * 2 + 0, j * 2 + 0, 0), this->generateColorU8(), this->generateColor(), this->generateColor(), this->generateColor());
			VertexData[Index * 4 + 1] = vertex(glm::vec3(i * 2 + 2, j * 2 + 0, 0), this->generateColorU8(), this->generateColor(), this->generateColor(), this->generateColor());
			VertexData[Index * 4 + 2] = vertex(glm::vec3(i * 2 + 2, j * 2 + 2, 0), this->generateColorU8(), this->generateColor(), this->generateColor(), this->generateColor());
			VertexData[Index * 4 + 3] = vertex(glm::vec3(i * 2 + 0, j * 2 + 2, 0), this->generateColorU8(), this->generateColor(), this->generateColor(), this->generateColor());
			ElementData[Index * 6 + 0] = Index * 4 + 0;
			ElementData[Index * 6 + 1] = Index * 4 + 1;
			ElementData[Index * 6 + 2] = Index * 4 + 2;
			ElementData[Index * 6 + 3] = Index * 4 + 2;
			ElementData[Index * 6 + 4] = Index * 4 + 3;
			ElementData[Index * 6 + 5] = Index * 4 + 0;
		}
*/

		//this->beginTimer();

/*
		//glBufferSubData(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(vertex), &this->VertexData[0]);
*/

/*
		for(std::size_t i = 0; i < VertexData.size(); ++i)
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex), &this->VertexData[0] + i);
*/

/* // 1 map
		vertex* pointer = reinterpret_cast<vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(vertex), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		memcpy(pointer, &VertexData[0], VertexData.size() * sizeof(vertex));
		glUnmapBuffer(GL_ARRAY_BUFFER);
*/

/* // N maps
		for(std::size_t i = 0; i < VertexData.size(); ++i)
		{
			vertex* pointer = reinterpret_cast<vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, i * sizeof(vertex), sizeof(vertex), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
			memcpy(pointer, &VertexData[0] + i, sizeof(vertex));
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}
*/

/*
		vertex* pointer = reinterpret_cast<vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(vertex), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
		memcpy(pointer, &VertexData[0], VertexData.size() * sizeof(vertex));
		glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(vertex));
		glUnmapBuffer(GL_ARRAY_BUFFER);
*/

		vertex* pointer = reinterpret_cast<vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(vertex), GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
		//glInvalidateBufferData(BufferName[buffer::VERTEX]);
		memcpy(pointer, &VertexData[0], VertexData.size() * sizeof(vertex));
		for(std::size_t i = 0; i < VertexData.size(); ++i)
		{
			//memcpy(pointer, &VertexData[0] + i, sizeof(vertex));
			glFlushMappedBufferRange(GL_ARRAY_BUFFER, i * sizeof(vertex), sizeof(vertex));
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);

/*
		vertex* pointer = reinterpret_cast<vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, VertexData.size() * sizeof(vertex), GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT));
		glInvalidateBufferData(BufferName[buffer::VERTEX]);
		const std::size_t Batch = 1;
		std::size_t Offset = 0;
		for(std::size_t i = 0; i < VertexData.size() / Batch; ++i)
		{
			memcpy(pointer, &VertexData[0] + i * Batch, sizeof(vertex) * Batch);

			if(i * sizeof(vertex) - Offset > 64 * 1024)
			{
				glFlushMappedBufferRange(GL_ARRAY_BUFFER, Offset, 64 * 1024);
				Offset += 64 * 1024;
			}
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
*/

			glDrawElementsInstanced(GL_TRIANGLES, this->ElementCount, GL_UNSIGNED_INT, 0, 1);
		//this->endTimer();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	test_buffer Test(argc, argv, 10000, glm::uvec2(512));

	csv CSV;

	Error += Test();

	Test.log(CSV, "GNI");
	CSV.save("../test-buffer.csv");

	return Error;
}

