VertexShader
{
	Node = WorldPos
	{
		Input = ''
		{
			Index = 0
		}
		Output
		{
		}
	}
}

FragmentShader
{
	Textures
	{
		Texture = DiffuseMap
		{
			Slot = 0
			Source = Material
		}
	}
	
	Layer
	{
		Type = Color
		Index = 0
		Input = Texture
		{
			Source=DiffuseMap
		}
	}
}