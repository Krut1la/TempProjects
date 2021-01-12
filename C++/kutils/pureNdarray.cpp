template<class T> 
void* newNdArray(int *dims, int dimCount)
{
	int bufferSize = 0;

	int tableSize = 1;
	for(int dim = 0; dim < dimCount - 1; ++dim)
	{
		tableSize *= dims[dim];
		bufferSize += sizeof(T*) * tableSize;
	}

	bufferSize += sizeof(T) * tableSize * dims[dimCount - 1];

#ifndef USEMALLOC
	void* buffer = new char[bufferSize];
#else
	void* buffer = malloc(bufferSize);
#endif

	if(!buffer) return 0;

	tableSize = 1;
	int tableOffset = 0;

	for(int dim = 0; dim < dimCount - 1; ++dim)
	{
		tableSize *= dims[dim];
		tableOffset += tableSize;

		for(int i = 0; i < tableSize; ++i)
		{
				if(dim < dimCount - 2)
				*((T***)buffer + tableOffset - tableSize + i) = (((T**)buffer + tableOffset + i*dims[dim + 1]));
				else
			    *((T**)buffer + tableOffset - tableSize + i) = ((T*)((T**)buffer + tableOffset) + i*dims[dim + 1]);
		}
	}

	return buffer;
}


template<class T>
void* newNdArray(int dimCount ...)
{
#ifndef USEMALLOC
    int* dims = new int[dimCount];
#else
	int* dims = (int*)malloc(sizeof(int)*dimCount];
#endif

	va_list ap;
	va_start(ap, dimCount);

	for(int i = 0; i < dimCount; ++i)
	    dims[i] = va_arg(ap, int);

	va_end(ap);

	void* buffer = newNdArray<T>(dims, dimCount);

#ifndef USEMALLOC
	delete[] dims;
#else
	free(dims);
#endif
	return buffer;
}