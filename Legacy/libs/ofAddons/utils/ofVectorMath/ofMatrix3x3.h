#ifndef _OF_MATRIX_3X3
#define _OF_MATRIX_3X3

/*
Credits: 
Code adopted from Lode Vandevenne http://student.kuleuven.be/~m0216922/CG/
*/


class ofMatrix3x3{

public:
	
	
    //----------------  Constructor
    
    
	/**
	* Constructor. 
	*
	* @param  _a - _i values of matix.
	* [ a b c ]                                                    
	* [ d e f ]                                                    
	* [ g h i ] 
	*/
    
    ofMatrix3x3( double _a, double _b, double _c, double _d, double _e, double _f, double _g, double _h, double _i )
    { 
      a = _a;
      b = _b;
      c = _c;
      d = _d;
      e = _e;
      f = _f;
      g = _g;
      h = _h;
      i = _i;
    }
	
    
    
	/**
	* Deafult Constructor. 
	*/
    
	ofMatrix3x3()
    {
    
      a = 0.0;
      b = 0.0;
      c = 0.0;
      d = 0.0;
      e = 0.0;
      f = 0.0;
      g = 0.0;
      h = 0.0;
      i = 0.0; 
	}
    
    
    
    //---------------- Data Members
    
    /**
    * [ a b c ]                                                    
    * [ d e f ]                                                    
    * [ g h i ] 
    */
    
      double a;
      double b;
      double c;
      double d;
      double e;
      double f;
      double g;
      double h;
      double i;
    
   
    //---------------- Methods
    
	/**
     * Transpose:                                                                  
     * This changes the matrix.                                                                           
     * [ a b c ]T    [ a d g ]                                                 
     * [ d e f ]  =  [ b e h ]         
     * [ g h i ]     [ c f i ]
     */
     
	void transpose()
	{
		b += d; d = b - d; b -= d; //swap b and d
		c += g; g = c - g; c -= g; //swap c and g
		f += h; h = f - h; f -= h; //swap f and h
	}  
    
	/**
	* Transpose without changing the matrix.
    * Uses the "swap" method with additions and subtractions to swap the elements that aren't on the main diagonal.
    * @return transposed matrix.
    */
      
	ofMatrix3x3 transpose(const ofMatrix3x3& A)
	{
		ofMatrix3x3 result = A;
		result.transpose();
		return result;
	} 
    
    
     
	/**
    * Determinant: http://mathworld.wolfram.com/Determinant.html
    */
    
    double determinant() const
    {
           double det = a * e * i 
                      + b * f * g 
                      + d * h * c 
                      - g * e * c 
                      - d * b * i 
                      - h * f * a;
           return det;
    }
    
    double determinant(const ofMatrix3x3& A)
    {
           return A.determinant();
    }

    
    
    /**
    * Inverse of a 3x3 matrix
      the inverse is the adjoint divided through the determinant
      find the matrix of minors (minor = determinant of 2x2 matrix of the 2 rows/colums current element is NOT in)
      turn them in cofactors (= change some of the signs)
      find the adjoint by transposing the matrix of cofactors
      divide this through the determinant to get the inverse
    */
    
    void invert()
    {
         double det = determinant();
         ofMatrix3x3 B;

         //included in these calculations: minor, cofactor (changed signs), transpose (by the order of "="), division through determinant
         B.a = ( e * i - h * f) / det;
         B.b = (-b * i + h * c) / det;
         B.c = ( b * f - e * c) / det;
         B.d = (-d * i + g * f) / det;
         B.e = ( a * i - g * c) / det;
         B.f = (-a * f + d * c) / det;
         B.g = ( d * h - g * e) / det;
         B.h = (-a * h + g * b) / det;
         B.i = ( a * e - d * b) / det;
  
         *this = B;
    }

    ofMatrix3x3 inverse(const ofMatrix3x3& A)
    {
      ofMatrix3x3 result = A;
      result.invert();
      return result;
    } 

    
    
    /** 
    * Add two matrices
    */
    ofMatrix3x3 operator+(const ofMatrix3x3& B)
    {
      ofMatrix3x3 result;
      result.a = a + B.a;
      result.b = b + B.b;
      result.c = c + B.c;
      result.d = d + B.d;
      result.e = e + B.e;
      result.f = f + B.f;
      result.g = g + B.g;
      result.h = h + B.h;
      result.i = i + B.i;
      return result;
    }
    
    void operator+=(const ofMatrix3x3& B)
    {
      a += B.a;
      b += B.b;
      c += B.c;
      d += B.d;
      e += B.e;
      f += B.f;
      g += B.g;
      h += B.h;
      i += B.i;
      
    }

    /** 
    * Subtract two matrices
    */
    
    ofMatrix3x3 operator-(const ofMatrix3x3& B)
    {
      ofMatrix3x3 result;
      result.a = a - B.a;
      result.b = b - B.b;
      result.c = c - B.c;
      result.d = d - B.d;
      result.e = e - B.e;
      result.f = f - B.f;
      result.g = g - B.g;
      result.h = h - B.h;
      result.i = i - B.i;
      return result;
    }
	
	void operator-=(const ofMatrix3x3& B)
    {
      a -= B.a;
      b -= B.b;
      c -= B.c;
      d -= B.d;
      e -= B.e;
      f -= B.f;
      g -= B.g;
      h -= B.h;
      i -= B.i;
      
    }
    
    
    /** 
    * Multiply a matrix with a scalar
    */
    
    ofMatrix3x3 operator*(double scalar)
    {
      ofMatrix3x3 result;
      result.a = a * scalar;
      result.b = b * scalar;
      result.c = c * scalar;
      result.d = d * scalar;
      result.e = e * scalar;
      result.f = f * scalar;
      result.g = g * scalar;
      result.h = h * scalar;
      result.i = i * scalar;
      return result;
    }
	
	
	void operator*=(const ofMatrix3x3& B)
    {
      a *= B.a;
      b *= B.b;
      c *= B.c;
      d *= B.d;
      e *= B.e;
      f *= B.f;
      g *= B.g;
      h *= B.h;
      i *= B.i;
      
    }
    
    void operator*=(double scalar)
    {
      a *= scalar;
      b *= scalar;
      c *= scalar;
      d *= scalar;
      e *= scalar;
      f *= scalar;
      g *= scalar;
      h *= scalar;
      i *= scalar;
      
    }
   
     /**
     * Multiply a 3x3 matrix with a 3x3 matrix
     */
    
    ofMatrix3x3 operator*(const ofMatrix3x3& B)
    {
      ofMatrix3x3 C;
      C.a = a * B.a + b * B.d + c * B.g;
      C.b = a * B.b + b * B.e + c * B.h;
      C.c = a * B.c + b * B.h + c * B.i;
      C.d = d * B.a + e * B.d + f * B.g;
      C.e = d * B.b + e * B.e + f * B.h;
      C.f = d * B.c + e * B.h + f * B.i;
      C.g = g * B.a + h * B.d + i * B.g;
      C.h = g * B.b + h * B.e + i * B.h;
      C.i = g * B.c + h * B.h + i * B.i;
      return C;
    }
     
   /** 
    * Divide a matrix through a scalar
    */
    ofMatrix3x3 operator/(double scalar)
    {
      ofMatrix3x3 result;
      result.a = a / scalar;
      result.b = b / scalar;
      result.c = c / scalar;
      result.d = d / scalar;
      result.e = e / scalar;
      result.f = f / scalar;
      result.g = g / scalar;
      result.h = h / scalar;
      result.i = i / scalar;
      return result;
    }

	
	void operator/=(const ofMatrix3x3& B)
    {
      a /= B.a;
      b /= B.b;
      c /= B.c;
      d /= B.d;
      e /= B.e;
      f /= B.f;
      g /= B.g;
      h /= B.h;
      i /= B.i;
      
    }
    
    void operator/=(double scalar)
    {
      a /= scalar;
      b /= scalar;
      c /= scalar;
      d /= scalar;
      e /= scalar;
      f /= scalar;
      g /= scalar;
      h /= scalar;
      i /= scalar;
      
    }
    



	
};

#endif
