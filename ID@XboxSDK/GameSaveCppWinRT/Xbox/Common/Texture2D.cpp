//--------------------------------------------------------------------------------------
// Texture2D.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Texture2D.h"

#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

using namespace DirectX;

Texture2D::Texture2D(ID3D11ShaderResourceView* resourceView)
{
	m_resourceView = resourceView;
	m_resourceView->AddRef();

	// Go through the resource view to the resource to get the description for this texture 2D object
	ID3D11Resource* resource;
    m_resourceView->GetResource(&resource);
	ID3D11Texture2D* texture2D;
	resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture2D);
	texture2D->GetDesc(&m_desc);
	texture2D->Release();
	resource->Release();
}

Texture2D::~Texture2D()
{
	if (m_resourceView)
        m_resourceView->Release();
}

void Texture2D::GetResource(ID3D11Resource** resource)
{
	m_resourceView->GetResource(resource);
}

void Texture2D::GetResourceView(ID3D11ShaderResourceView** resourceView)
{
	*resourceView = m_resourceView;
	(*resourceView)->AddRef();
}

ID3D11ShaderResourceView* Texture2D::GetResourceViewTemporary()
{
	return m_resourceView;
}

std::unique_ptr<Texture2D> Texture2D::FromFile(ID3D11Device* device, const std::wstring& file)
{
	// Figure out whether or not we're loading a DDS file
	bool isDDS = false;
	{
		std::wstring ddsExtension(L"dds");
		if (file.length() >= ddsExtension.length()) 
		{
			isDDS = (0 == file.compare(file.length() - ddsExtension.length(), ddsExtension.length(), ddsExtension));
		} 
		else
		{
			isDDS = false;
		}
	}

	// Load the texture with the appropriate method
	ID3D11ShaderResourceView* resourceView = nullptr;
	if (isDDS)
	{
        CreateDDSTextureFromFile(device, file.c_str(), nullptr, &resourceView);
    }
	else
	{
        CreateWICTextureFromFile(device, file.c_str(), nullptr, &resourceView);
    }

	// Create the texture from the resource view
	std::unique_ptr<Texture2D> texture(new Texture2D(resourceView));

	// Release the resource view
	resourceView->Release();

	// Hand back the texture object
	return texture;
}