#include "Shader.h"


Shader::Shader(std::string_view name)
{
	// Get Fullpath to cso object
	static std::filesystem::path shaderDir;
	if (shaderDir.empty())
	{
		wchar_t moduleFileName[MAX_PATH];
		GetModuleFileNameW(nullptr, moduleFileName, MAX_PATH);

		shaderDir = moduleFileName;
		shaderDir.remove_filename();
	}

	std::ifstream shaderIn(shaderDir / name, std::ios::binary);

	// Read entire contents of a file into memory
	if (shaderIn.is_open())
	{
		shaderIn.seekg(0, std::ios::end);
		m_size = shaderIn.tellg();
		shaderIn.seekg(0, std::ios::beg);
		m_data = malloc(m_size);
		if (m_data)
		{
			shaderIn.read((char*)m_data, m_size);
		}
	}

}

Shader::~Shader()
{
	if (m_data)
	{
		free(m_data);
	}
}
