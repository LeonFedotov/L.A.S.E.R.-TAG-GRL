#ifndef _OF_VEC4f
#define _OF_VEC4f


class ofVec4f{

public:
	
	
    //----------------  Constructors
    
    
    /**
     * Constructor. Defaults to zero,zero,zero,zero.
     *
     * @param  _x initial x coordinate. R for color notation. 
     * @param  _y initial y coordinate. G for color notation.
     * @param  _z initial z coordinate. B for color notation.
     * @param  _w initial w coordinate. A (alpha) for color notation.
     */
    ofVec4f( const float _x = 0.0f, const float _y = 0.0f, const float _z = 0.0f, const float _w = 0.0f )
        { x = _x;  y = _y;  z = _z; w = _w; }
	
    /**
     * Constructor. Copies value in vec.
     *
     * @param  vec source of initial coordinates to copy.
     */
    ofVec4f( const ofVec4f &vec )  
        { x = vec.x;  y = vec.y;  z = vec.z; w = vec.w;}
    
    
    
    //---------------- Data Members
    
    /**
     * x-coordinate (R)
    */
    float x;
    
    /**
     * y-coordinate (G)
    */
    float y;
    
    /**
     * z-coordinate (B)
    */
    float z;
    
    /**
     * z-coordinate (Alpha)
    */
    float w;
    
    //---------------- Methods
    
    /**
     * Set x,y,z,w coordinates of vector.
     *
     * @param  _x  x coordinate.
     * @param  _y  y coordinate.
     * @param  _z  z coordinate.
     * @param  _w  w coordinate.
     */
    void set( 	float _x, float _y, float _z, float _w )
        { x = _x;  y = _y;  z = _z; w = _w; }
    
    /**
     * Set x,y,z,w coordinates to the values in a vector object.
     *
     * @param vec  vector object to copy.
     */
     void set( const ofVec4f &vec )
       { x = vec.x;  y = vec.y;  z = vec.z; w = vec.w; }

    
    
    /**
	  * Array access operator
	  * @param i Array index
	  * @return For i = 0, reference to x coordinate, i = 1
	  * reference to y, i = 2 reference to z, i = 3 reference to w else
	  * x coordinate.
	  */
    float &operator[]( const int &i )           
        { switch(i) {
          case 0:  return x;
          case 1:  return y;
          case 2:  return z;
          case 3:  return w;
          default: return x; } }

    
    
    /**
     * Addition operator.
     * @param vec the vector to add
     * @return this + vec
     */  
    ofVec4f operator+( const ofVec4f & vec ) const          
    { 
        return ofVec4f( x+vec.x, y+vec.y, z+vec.z, w+vec.w); 
    } 
    
    /**
     * Add a vector into this
     * @param vec the vector to add
     * @return this 
     */
    ofVec4f &operator+=( const ofVec4f & vec )              
    { 
        x+=vec.x; y+=vec.y; z+=vec.z; w+=vec.w;
        return *this; 
    } 

    /**
     * Subtraction operator.
     * @return -this
     */ 
    ofVec4f operator-() const                               
    { 
        return ofVec4f(-x, -y, -z, -w); 
    } 
    
    /**
     * Subtraction operator.
     * @param vec to subtract
     * @return this - vec
     */ 
    ofVec4f operator-( const ofVec4f & vec ) const          
    { 
        return ofVec4f(x-vec.x, y-vec.y, z-vec.z, w-vec.w); 
    } 
    
    /**
     * Subtract a vector from this.
     * @param vec to subtract
     * @return this 
     */ 
    ofVec4f &operator-=( const ofVec4f & vec )              
    { 
        x-=vec.x; y-=vec.y; z-=vec.z; w-=vec.w;
        return *this; 
    } 
    
    
    /**
     * Multiplication operator.
     * @param vec to multiply by
     * @return this * vec
     */ 
    ofVec4f operator*( const ofVec4f & vec ) const
    { 
        return ofVec4f(x*vec.x, y*vec.y, z*vec.z, w*vec.w); 
    } 
    
    /**
     * Multiplication operator.
     * @param vec to multiply by
     * @return this
     */ 
     ofVec4f &operator*=( const ofVec4f & vec )      
    { 
        x*=vec.x; y*=vec.y; z*=vec.z; w*=vec.w;
        return *this; 
    } 

    
    /**
     * Scalar Multiplication.
     * @param f value to multiply by 
     * @param vec vector to scale
     * @return vector scaled by value in f
     */ 
    friend ofVec4f operator*( const float f, const ofVec4f &vec )  
    { 
        return ofVec4f(vec.x*f, vec.y*f, vec.z*f, vec.w*f); 
    } 
    
    
    /**
     * Division operator.
     * @param vec to divide by
     * @return this / vec
     */ 
    ofVec4f operator/( const ofVec4f & vec ) const  
    { 
        return ofVec4f(x/vec.x, y/vec.y, z/vec.z, w/vec.w);
    } 
    
    /**
     * Division operator.
     * @param vec to divide by
     * @return this
     */ 
    ofVec4f &operator/=( const ofVec4f & vec )      
    { 
        x/=vec.x; y/=vec.y; z/=vec.z; w/=vec.w;
        return *this; 
    } 

    /**
     * Scalar Multiplication. 
     * @param f value to multiply by. 
     * @return value of this scaled by value in f
     */ 
    ofVec4f operator*( const float f ) const                
    { 
        return ofVec4f(x*f, y*f, z*f, w*f);
    } 
    
    
    
    /**
     * Scalar Multiplication.
     * @param f value to multiply by 
     * @return this
     */ 
     ofVec4f &operator*=( const float f )                    
    { 
        x*=f; y*=f; z*=f; w*=f;
        return *this; 
    } 

    
    /**
     * Scalar Division. 
     * @param f value to divide by. 
     */ 
    ofVec4f operator/( const float f ) const      
    { 
        return ofVec4f(x/f, y/f, z/f, w/f);
    } 
     /**
     * Scalar Division. 
     * @param f value to divide by. 
     */ 
    ofVec4f &operator/=( const float f )          
    { 
        x/=f; y/=f; z/=f; w/=f;
        return *this; 
    } 


    /**
     * Calculate the length (magnitude) of the vector. 
     * @return length
    */
    float length() const                
    { 
        return (float)sqrt(x*x + y*y + z*z + w*w); 
    } 
    
    /**
     * Calculate the squared length of the vector. 
     * This should be faster than length()
     * @return length squared
    */
    float lengthSquared() const                
    { 
        return (float)(x*x + y*y + z*z + w*w); 
    } 
    
    /**
     * Normalizes this 
     * @return this
    */
    ofVec4f &normalize()                  
    { 
			float f = length();
			if( f > 0){ 
				x/=f; 
				y/=f; 
				z/=f;
				w/=f;
			}
        
		  return *this; 
	} 
    
    /**
     * Calculate s normal without modifying this. 
     * @return normalized value
    */
    ofVec4f norm_value() const 
    { 
        float f = length();
        if( f > 0) 
            return ofVec4f(x/f, y/f, z/f, w/f); 
        else 
            return ofVec4f(0, 0, 0, 0); 
    } 

	/**
     * Limit the length of this vector
     * @param max the maximum length to limit this vector by
     */  
	 ofVec4f limit(float max) {
		if (length() > max) {
			normalize();
			x*=max; y*=max; z*=max; w*=max;
		}
		return *this; 
	}
	
	
    
    

	
};

#endif
