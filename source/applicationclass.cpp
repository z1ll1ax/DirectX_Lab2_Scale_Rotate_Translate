////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_Model2 = 0;
	m_Light = 0;
	m_ShaderManager = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char sphereFilename[128], cubeFilename[128], textureFilename1[128], textureFilename2[128];
	bool result;


	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, "Could not initialize Direct3D", "Error", MB_OK);
		return false;
	}

	// Create and initialize the camera object.
	m_Camera = new CameraClass;

	m_Camera->SetPosition(0.0f, 0.0f, -8.0f);
	m_Camera->Render();

	// Set the file name of the model.
    strcpy_s(sphereFilename, "C:\\Users\\Владислав\\Desktop\\laba\\data\\sphere.txt");
	strcpy_s(cubeFilename, "C:\\Users\\Владислав\\Desktop\\laba\\cube.txt");
    // Set the file name of the textures.
	strcpy_s(textureFilename1, "C:\\Users\\Владислав\\Desktop\\laba\\data\\wood_texture.tga");
    strcpy_s(textureFilename2, "C:\\Users\\Владислав\\Desktop\\laba\\data\\wood_normal_map.tga");

    // Create and initialize the model object.
    m_Model = new ModelClass;
	m_Model2 = new ModelClass;

    result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), sphereFilename, textureFilename1, textureFilename2);
    if(!result)
    {
        return false;
    }
	result = m_Model2->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), cubeFilename, textureFilename1, textureFilename2);
	if (!result)
	{
		return false;
	}

	 // Create and initialize the light object.
    m_Light = new LightClass;

    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	// Create and initialize the normal map shader object.
	m_ShaderManager = new ShaderManagerClass;

	result = m_ShaderManager->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	if(m_ShaderManager)
	{
		m_ShaderManager->Shutdown();
		delete m_ShaderManager;
		m_ShaderManager = 0;
	}

    if(m_Light)
    {
        delete m_Light;
        m_Light = 0;
    }

    if(m_Model)
    {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = 0;
    }

	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}


bool ApplicationClass::Frame(InputClass* Input)
{
	static float rotation = 360.0f;
	static float position = 3.0f;
	static float scale = 1.0f;
	bool result;

	if(Input->IsEscapePressed())
	{
		return false;
	}

	position -= 0.025f;
	if (position <= -3.0f)
	{
		position += 6.0f;
	}

    rotation -= 0.0174532925f * 0.25f;
    if(rotation <= 0.0f)
    {
        rotation += 360.0f;
    }

	scale += 0.005f;
		if (scale >= 1.0f)
		{
			scale -= 1.0f;
		}
	
	result = Render(position, rotation, scale);
	if(!result)
	{
		return false;
	}

	return true;
}


bool ApplicationClass::Render(float position, float rotation, float scale)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, rotateMatrix, translateMatrix, scaleMatrix, srMatrix;
	bool result;

	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	rotateMatrix = XMMatrixRotationY(rotation);
	translateMatrix = XMMatrixTranslation(-3.0f, 2.0f, 0.0f);
	scaleMatrix = XMMatrixScaling(scale, scale, scale);
	srMatrix = XMMatrixMultiply(scaleMatrix, rotateMatrix);
	worldMatrix = XMMatrixMultiply(srMatrix, translateMatrix);
	m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_ShaderManager->RenderTextureShader(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
												  m_Model->GetTexture(0));
	if(!result)
	{
		return false;
	}

	rotateMatrix = XMMatrixRotationY(rotation);
	translateMatrix = XMMatrixTranslation(position, -1.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(rotateMatrix, translateMatrix);
	m_Model2->Render(m_Direct3D->GetDeviceContext());
	result = m_ShaderManager->RenderNormalMapShader(m_Direct3D->GetDeviceContext(), m_Model2->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model2->GetTexture(0), m_Model2->GetTexture(1), m_Light->GetDirection(), m_Light->GetDiffuseColor());

	if (!result)
	{
		return false;
	}

	rotateMatrix = XMMatrixRotationY(rotation);
	translateMatrix = XMMatrixTranslation(3.0f, 2.0f, 0.0f);
	scaleMatrix = XMMatrixScaling(scale, 1.0f, scale);
	srMatrix = XMMatrixMultiply(scaleMatrix, rotateMatrix);
	worldMatrix = XMMatrixMultiply(srMatrix, translateMatrix);
	m_Model->Render(m_Direct3D->GetDeviceContext());

	result = m_ShaderManager->RenderNormalMapShader(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
													m_Model->GetTexture(0), m_Model->GetTexture(1), m_Light->GetDirection(), m_Light->GetDiffuseColor());
	if(!result)
	{
		return false;
	}

	rotateMatrix = XMMatrixRotationX(rotation);
	translateMatrix = XMMatrixTranslation(0.0f, 1.5f, 0.0f);
	worldMatrix = XMMatrixMultiply(rotateMatrix, translateMatrix);
	m_Model2->Render(m_Direct3D->GetDeviceContext());
	result = m_ShaderManager->RenderNormalMapShader(m_Direct3D->GetDeviceContext(), m_Model2->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model2->GetTexture(0), m_Model2->GetTexture(1), m_Light->GetDirection(), m_Light->GetDiffuseColor());

	if (!result)
	{
		return false;
	}

	m_Direct3D->EndScene();

	return true;
}