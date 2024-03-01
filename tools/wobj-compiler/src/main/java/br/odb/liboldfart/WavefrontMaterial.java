package br.odb.liboldfart;

import br.odb.gameutils.Color;
import br.odb.libstrip.Material;

/**
 * Created by monty on 12/03/16.
 */
public class WavefrontMaterial {

	Material material;
	public final String name;

	WavefrontMaterial(String name) {
		this.name = name;
	}

	public void addColor(Color color) {
		String texture = null;

		if ( material != null ) {
			texture = material.texture;
		}

		if ( texture != null ) {
			material = Material.makeWithColorAndTexture( color, texture );
		} else {
			material = Material.makeWithColor( color );
		}
	}

	public void addTexture(String texture) {
		Color color = null;

		if ( material != null ) {
			color = material.mainColor;
		}

		if ( color != null ) {
			material = Material.makeWithColorAndTexture( color, texture );
		} else {
			material = Material.makeWithColor( color );
		}
	}
}
