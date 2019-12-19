#include "Graphics.h"



Graphics::Graphics()
{
	m_Direct3D = nullptr;
	m_Camera = nullptr;
	m_ModelColor = nullptr;
	m_ModelTexture = nullptr;
	m_ColorShader = nullptr;
	m_TextureShader = nullptr;
}


Graphics::Graphics(const Graphics& other)
{
}


Graphics::~Graphics()
{
}


bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;


	// Create the Direct3D object.
	m_Direct3D = new D3D;
	if (!m_Direct3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize Direct3D", "Error", MB_OK);
	}

	// Create the camera object.
	m_Camera = new Camera;
	if (!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// Create the model object.
	m_ModelColor = new Model;
	if (!m_ModelColor)
	{
		return false;
	}

	// Initialize the model object.
	result = m_ModelColor->InitializeWithColor(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the model with color object.", "Error", MB_OK);
		return false;
	}

	m_ModelTexture = new Model;
	if (!m_ModelTexture)
	{
		return false;
	}

	result = m_ModelTexture->InitializeWithTexture(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), "../DirectX11Tut/data/stone01.tga");
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the model with texture object.", "Error", MB_OK);
		return false;
	}

	// Create the texture shader object.
	m_TextureShader = new TextureShader;
	if (!m_TextureShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the color shader object.", "Error", MB_OK);
		return false;
	}

	// Create the color shader object.
	m_ColorShader = new ColorShader;
	if (!m_ColorShader)
	{
		return false;
	}

	// Initialize the color shader object.
	result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the color shader object.", "Error", MB_OK);
		return false;
	}

	char cardName[128];
	int memory;

	m_Direct3D->GetVideoCardInfo(cardName, memory);

	std::ofstream stream;
	stream.open("out.txt");
	stream << "Card name: " << cardName << std::endl;
	stream << "Card memory: " << memory << " MB" << std::endl;
	stream.close();

	return result;
}


void Graphics::Shutdown()
{
	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = nullptr;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the color shader object.
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = nullptr;
	}

	// Release the model object.
	if (m_ModelColor)
	{
		m_ModelColor->Shutdown();
		delete m_ModelColor;
		m_ModelColor = nullptr;
	}

	// Release the model object.
	if (m_ModelTexture)
	{
		m_ModelTexture->Shutdown();
		delete m_ModelTexture;
		m_ModelTexture = nullptr;
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = nullptr;
	}

	return;
}


bool Graphics::Frame()
{
	bool result;


	// Render the graphics scene.
	result = Render();

	return result;
}


bool Graphics::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);


	bool color = false;

	if (color)
	{
		// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
		m_ModelColor->Render(m_Direct3D->GetDeviceContext());

		// Render the model using the color shader.
		result = m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_ModelColor->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
		if (!result)
		{
			return false;
		}
	}
	else
	{
		// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
		m_ModelTexture->Render(m_Direct3D->GetDeviceContext());

		// Render the model using the color shader.
		result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_ModelTexture->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_ModelTexture->GetTexture());
		if (!result)
		{
			return false;
		}
	}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}