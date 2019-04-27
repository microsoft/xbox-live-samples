//--------------------------------------------------------------------------------------
// Texture2D.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <d3d11_x.h>
#include <DirectXMath.h>
#include <memory>
#include <string>

namespace DirectX
{
	class Texture2D
	{
	public:
		explicit Texture2D(ID3D11ShaderResourceView* resourceView);
		virtual ~Texture2D();

		inline int Width() const { return m_desc.Width; }
		inline int Height() const { return m_desc.Height; }
		inline int MipLevels() const { return m_desc.MipLevels; }
		inline DXGI_FORMAT Format() const { return m_desc.Format; }

		void GetResource(ID3D11Resource** resource);
		void GetResourceView(ID3D11ShaderResourceView** resourceView);

		ID3D11ShaderResourceView* GetResourceViewTemporary();

        static std::unique_ptr<Texture2D> FromFile(ID3D11Device* device, const std::wstring& file);

	private:
		ID3D11ShaderResourceView* m_resourceView;
		D3D11_TEXTURE2D_DESC m_desc;

        // Prevent copying.
        Texture2D(Texture2D const&);
        Texture2D& operator= (Texture2D const&);
	};
}