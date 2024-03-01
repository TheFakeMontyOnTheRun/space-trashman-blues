package br.odb.gameutils.math;

import java.io.Serializable;

import br.odb.gameutils.Utils;

/**
 * 
 * @author Daniel "Monty" Monteiro
 * 
 */

public class Vec3 implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 688571051366412593L;
	/**
	 * 
	 */
	public float x;
	/**
	 * 
	 */
	public float y;
	/**
	 * 
	 */
	public float z;

	/**
	 * 
	 */
	public Vec3() {
		this(0, 0, 0);
	}

	/**
	 * 
	 * @param v
	 */
	public Vec3(Vec3 v) {
		copy(v);
	}

	/**
	 * 
	 * @param x2
	 * @param y2
	 * @param z2
	 */
	public Vec3(float x2, float y2, float z2) {
		x = x2;
		y = y2;
		z = z2;
	}

	@Override
	public String toString() {
		String toReturn = "";
		toReturn += x;
		toReturn += " ";
		toReturn += y;
		toReturn += " ";
		toReturn += z;
		return toReturn;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + Float.floatToIntBits(x);
		result = prime * result + Float.floatToIntBits(y);
		result = prime * result + Float.floatToIntBits(z);
		return result;
	}

	/**
	 * 
	 */
	@Override
	public boolean equals(Object that) {

		if (this == that)
			return true;
		if (!(that instanceof Vec3))
			return false;
		Vec3 v = (Vec3) that;

		return Utils.eqFloat(v.x, x) && Utils.eqFloat(v.y, y)
				&& Utils.eqFloat(v.z, z);
	}

	public float length() {
		return (float) Math.sqrt((x * x) + (y * y) + (z * z));
	}

	public void normalize() {

		float oneOverLen = 1.0f / length();
		scale(oneOverLen);
	}

	public void scale(float real) {

		x *= real;
		y *= real;
		z *= real;
	}

	public Vec3 sub(Vec3 place) {

		Vec3 toReturn = new Vec3();
		toReturn.x = (x - place.x);
		toReturn.y = (y - place.y);
		toReturn.z = (z - place.z);

		return toReturn;
	}

	public Vec3 add(Vec3 delta) {

		Vec3 toReturn = new Vec3();

		toReturn.x = (x + delta.x);
		toReturn.y = (y + delta.y);
		toReturn.z = (z + delta.z);

		return toReturn;
	}

	public void copy(Vec3 place) {

		set(place.x, place.y, place.z);
	}

	public void set(float _x, float _y, float _z) {
		x = (_x);
		y = (_y);
		z = (_z);
	}

	public void set(Vec3 v) {
		
		if ( v == null || !v.isValid() ) {
			return;
		}
		
		set(v.x, v.y, v.z);
	}

	public Vec3 normalized() {

		Vec3 length1 = new Vec3(this);
		length1.normalize();
		return length1;
	}

	public void addTo(Vec3 v) {

		x += v.x;
		y += v.y;
		z += v.z;
	}

	public void addTo(float x2, float y2, float z2) {

		x += x2;
		y += y2;
		z += z2;
	}

	public Vec3 crossProduct(Vec3 v2) {

		Vec3 o = new Vec3();

		o.x = (y * v2.z) - (v2.y * z);
		o.y = (z * v2.x) - (v2.z * x);
		o.z = (x * v2.y) - (v2.x * y);

		return o;
	}

	public float dotProduct(Vec3 v2) {
		return (x * v2.x) + (y * v2.y) + (z * v2.z);
	}

	public boolean isValid() {

		return !(Float.isInfinite(x) || Float.isNaN(x) || Float.isInfinite(y)
				|| Float.isNaN(y) || Float.isInfinite(z) || Float.isNaN(z));
	}
	
	public boolean isZero() {
		//Yeah, I know it's a mistake to compare floats like this - but in my defence, even the slightest fraction means it's not zero.
		return x == 0.0f && y == 0.0f && z == 0.0f;
	}
	
	public Vec3 scaled( float t ) {
		
		Vec3 toReturn = new Vec3( this );
		toReturn.scale( t );
		
		return toReturn;
	}
}
