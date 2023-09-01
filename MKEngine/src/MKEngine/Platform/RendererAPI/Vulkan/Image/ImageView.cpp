#include "mkpch.h"

#include "ImageView.h"

namespace MKEngine {
    ImageView ImageView::CreateImageView(ImageViewDescription description)
    {
        ImageView imageView;

        if(description.IsSwapchain)
        {
            imageView.Image = description.Image;
        }else
        {
	        
        }

        return ImageView();
    }
}