package br.odb.libstrip;

import br.odb.gameutils.Direction;
import br.odb.gameutils.math.Vec3;

import java.io.Serializable;

public class GeneralTriangle implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 9168091717363956101L;

	public Direction hint;
	
	public Material material;
	
	public float x0;
	public float y0;
	public float z0;
	public float x1;
	public float y1;
	public float z1;
	public float x2;
	public float y2;
	public float z2;
	public float nx;
	public float ny;
	public float nz;
	private float[] mTextureCoordinates =  new float[]{ 0.0f, 0.0f,
														0.0f, 0.0f,
														0.0f, 0.0f
														};

	public void flush() {
	}

	public float[] getVertexData() {
		return new float[] { x0, y0, z0, x1, y1, z1, x2, y2, z2 };
	}

	public float[] getTextureCoordinates() { return mTextureCoordinates; }

	public void setTextureCoordinates(float[] textureCoordinates) { mTextureCoordinates = textureCoordinates; }

	public GeneralTriangle makeCopy() {
		
		GeneralTriangle toReturn = new GeneralTriangle();
		
		toReturn.x0 = x0;
		toReturn.x1 = x1;
		toReturn.x2 = x2;
		toReturn.y0 = y0;
		toReturn.y1 = y1;
		toReturn.y2 = y2;
		toReturn.z0 = z0;
		toReturn.z1 = z1;
		toReturn.z2 = z2;
		
		toReturn.material = material;
		
		return toReturn;
	}

	public Vec3 getVertex(int c) {
		switch ( c ) {
		case 0:
			return new Vec3( x0, y0, z0 );
		case 1:
			return new Vec3( x1, y1, z1);
		case 2:
		default:
			return new Vec3( x2, y2, z2 );
		}		
	}

	public Vec3 makeNormal() {
		Vec3 v1 = new Vec3( x1 - x0, y1 - y0, z1 - z0 );
		Vec3 v2 = new Vec3( x2 - x0, y2 - y0, z2 - z0 );
		return v1.crossProduct( v2 ).normalized();
	}

	public void flatten(float z) {
		z0 = z1 = z2 = z;		
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result
				+ ((material == null) ? 0 : material.hashCode());
		result = prime * result + Float.floatToIntBits(x0);
		result = prime * result + Float.floatToIntBits(x1);
		result = prime * result + Float.floatToIntBits(x2);
		result = prime * result + Float.floatToIntBits(y0);
		result = prime * result + Float.floatToIntBits(y1);
		result = prime * result + Float.floatToIntBits(y2);
		result = prime * result + Float.floatToIntBits(z0);
		result = prime * result + Float.floatToIntBits(z1);
		result = prime * result + Float.floatToIntBits(z2);
		return result;
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof GeneralTriangle)) {
			return false;
		}
		GeneralTriangle other = (GeneralTriangle) obj;
		if (material == null) {
			if (other.material != null) {
				return false;
			}
		} else if (!material.equals(other.material)) {
			return false;
		}
		if (Float.floatToIntBits(x0) != Float.floatToIntBits(other.x0)) {
			return false;
		}
		if (Float.floatToIntBits(x1) != Float.floatToIntBits(other.x1)) {
			return false;
		}
		if (Float.floatToIntBits(x2) != Float.floatToIntBits(other.x2)) {
			return false;
		}
		if (Float.floatToIntBits(y0) != Float.floatToIntBits(other.y0)) {
			return false;
		}
		if (Float.floatToIntBits(y1) != Float.floatToIntBits(other.y1)) {
			return false;
		}
		if (Float.floatToIntBits(y2) != Float.floatToIntBits(other.y2)) {
			return false;
		}
		if (Float.floatToIntBits(z0) != Float.floatToIntBits(other.z0)) {
			return false;
		}
		return Float.floatToIntBits(z1) == Float.floatToIntBits(other.z1) && Float.floatToIntBits(z2) == Float.floatToIntBits(other.z2);
	}
}
