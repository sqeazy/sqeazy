package sqeazy.bindings.test;

import static org.junit.Assert.assertEquals;

import org.bridj.CLong;
import org.bridj.Pointer;
import org.junit.Test;

import sqeazy.bindings.SqeazyLibrary;

public class SqeazyLibraryTests
{

	@Test
	public void testLZ4()
	{
		final int lBufferLength = 1024;

		final Pointer<Byte> lSourceBytes = Pointer.allocateBytes(lBufferLength);
		for (int i = 0; i < lBufferLength; i++)
			lSourceBytes.set(i, (byte) i);

		final Pointer<Byte> lDestinationBytes = Pointer.allocateBytes((long) (lBufferLength * 1.1));
		final Pointer<CLong> lPointerToDestinationLength = Pointer.allocateCLong();

		assertEquals(	0,
									SqeazyLibrary.SQY_LZ4Encode(lSourceBytes,
																							lSourceBytes.getValidBytes(),
																							lDestinationBytes,
																							lPointerToDestinationLength));

		final long lCompresssedBufferLength = lPointerToDestinationLength.getCLong();

		System.out.println("source buffer length: " + lBufferLength);
		System.out.println("destination buffer length: " + lCompresssedBufferLength);

		assertEquals(282, lCompresssedBufferLength);

		assertEquals(	0,
									SqeazyLibrary.SQY_LZ4_Decompressed_Length(lDestinationBytes,
																														lPointerToDestinationLength));

		System.out.println("destination buffer length accoding to SQY_LZ4_Decompressed_Length: " + lPointerToDestinationLength.getCLong());

		assertEquals(	lBufferLength,
									lPointerToDestinationLength.getCLong());

		final Pointer<Byte> lDecodedBytes = Pointer.allocateBytes(lBufferLength);

		assertEquals(0, SqeazyLibrary.SQY_LZ4Decode(lDestinationBytes,
																								lBufferLength,
																								lDecodedBytes));

		for (int i = 0; i < lBufferLength; i++)
		{
			final Byte lByte = lDecodedBytes.get(i);
			if ((byte) i != lByte)
				System.out.format("i=%d, decompressed: %d, should be: %d \n",
													i,
													lByte,
													(byte) i);
			assertEquals((byte) i, lByte, 0);
		}

	}

}
