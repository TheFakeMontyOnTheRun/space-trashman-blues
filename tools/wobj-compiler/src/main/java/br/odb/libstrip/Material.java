package br.odb.libstrip;

import br.odb.gameutils.Color;

import java.io.Serializable;

public class Material implements Serializable {

	/**
	 * This is a two-classes-in-one scenario. Must split the two.
	 */
	private static final long serialVersionUID = -6359681992594825126L;

	public final Color mainColor = new Color();
	public final String texture;


	public static Material makeWithColor(Color c) {
		return new Material(c, null);
	}

	public static Material makeWithTexture(String textureName) {
		return new Material(null, textureName);
	}

	public static Material makeWithColorAndTexture(Color c, String textureName) {
		return new Material(c, textureName);
	}

	private Material(Color c, String texture) {
		this.texture = texture;
		mainColor.set(c);
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof Material)) {
			return false;
		}
		Material other = (Material) obj;

		if (!mainColor.equals(other.mainColor)) {
			return false;
		}

		if (texture == null) {
			if (other.texture != null) {
				return false;
			}
		} else if (!texture.equals(other.texture)) {
			return false;
		}
		return true;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result
				+ (mainColor.hashCode());
		result = prime * result + ((texture == null) ? 0 : texture.hashCode());
		return result;
	}

	@Override
	public String toString() {

		StringBuilder sb = new StringBuilder("");

		sb.append("<material ");
		sb.append("mainColor='").append(mainColor.getHTMLColor()).append("' ");

		if (texture != null) {
			sb.append("texture = '").append(texture).append("' ");
		}

		sb.append(" />");

		return sb.toString();
	}
}
