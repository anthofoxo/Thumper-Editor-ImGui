project "aurora"
debugdir "../working"
kind "ConsoleApp"

files {
	"%{prj.location}/**.cpp",
	"%{prj.location}/**.cc",
	"%{prj.location}/**.c",
	"%{prj.location}/**.hpp",
	"%{prj.location}/**.h",
	"%{prj.location}/**.inl",
}

includedirs {
	"%{prj.location}",
	"%{prj.location}/source",
	"%{prj.location}/vendor",

	"%{wks.location}/vendor/glfw/include",
	"%{wks.location}/vendor/glad/include",
	"%{wks.location}/vendor/imgui",
	"%{wks.location}/vendor/tinyfd",
	"%{wks.location}/vendor/yaml/include",
}

defines "YAML_CPP_STATIC_DEFINE"
links { "glfw", "glad", "imgui", "tinyfd", "yaml" }

filter "system:windows"
files "%{prj.location}/*.rc"

filter "system:linux"
links { "pthread", "dl", "m" }