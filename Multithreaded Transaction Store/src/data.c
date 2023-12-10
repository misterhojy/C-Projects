#include "data.h"
#include "debug.h"
#include "csapp.h"
#include "helper.h"


/*
* Create a blob with given content and size.
* The content is copied, rather than shared with the caller.
* The returned blob has one reference, which becomes the caller's
* responsibility.
*
* @param content  The content of the blob.
* @param size  The size in bytes of the content.
* @return  The new blob, which has reference count 1.
*/
BLOB *blob_create(char *content, size_t size) {

   BLOB *created_blob = Malloc(sizeof(BLOB));
   pthread_mutex_init(&created_blob->mutex, NULL);

   if (size == 0) {
       created_blob->content = NULL;
       created_blob->prefix = NULL;
   } else {
       created_blob->prefix = Calloc(1, size + 1);
       memcpy(created_blob->prefix, content, size);
      
       created_blob->content = Calloc(1, size);
       memcpy(created_blob->content, content, size);
   }

   created_blob->size = size;
   created_blob->refcnt = 1;

  //  fprintf(stderr, "New Blob of %s\n", created_blob->prefix);

   return created_blob;
}


/*
* Increase the reference count on a blob.
*
* @param bp  The blob.
* @param why  Short phrase explaining the purpose of the increase.
* @return  The blob pointer passed as the argument.
*/
BLOB *blob_ref(BLOB *bp, char *why) {
  Pthread_mutex_lock(&bp->mutex); /* lock mutex*/

  bp->refcnt++;
  // fprintf(stderr, "INCREASING REF of %s: %d\n",bp->prefix ,bp->refcnt);
  // fprintf(stderr,"reason: %s\n", why);    //COMMENT OUT

  Pthread_mutex_unlock(&bp->mutex);       /* Unlock mutex */
  return bp;
}


/*
* Decrease the reference count on a blob.
* If the reference count reaches zero, the blob is freed.
*
* @param bp  The blob.
* @param why  Short phrase explaining the purpose of the decrease.
*/
void blob_unref(BLOB *bp, char *why) {
  Pthread_mutex_lock(&bp->mutex); /* lock mutex*/

  bp->refcnt--;
  // fprintf(stderr, "DECREASING REF of %s: %d\n",bp->prefix ,bp->refcnt);
  // fprintf(stderr,"WHY: %s\n", why);    //COMMENT OUT

  if (bp->refcnt == 0) {
      // fprintf(stderr, "REF COUNT of %s is 0 FREE IT\n", bp->prefix);
          Free(bp->content);
          Free(bp->prefix);

          Pthread_mutex_unlock(&bp->mutex);       /* Unlock mutex */
          Free(bp);
  } else {
      Pthread_mutex_unlock(&bp->mutex);       /* Unlock mutex */
  }
}


/*
* Compare two blobs for equality of their content.
*
* @param bp1  The first blob.
* @param bp2  The second blob.
* @return 0 if the blobs have equal content, nonzero otherwise.
*/
int blob_compare(BLOB *bp1, BLOB *bp2) {
  /* compare contents */
  if (bp1->size == bp2->size) {
      // fprintf(stderr, "BOTH BLOBS IN COMPARE SAME SIZE\n");
      int result = strcmp(bp1->content, bp2->content);
      // fprintf(stderr, "RESULT of blob compare: %d\n", result);
      return result;
  } else {
      if (bp1->size > bp2->size) {
          // fprintf(stderr, "RESULT of content compare: %ld\n", bp1->size);
          return (bp1->size - bp2->size);
      } else {
          // fprintf(stderr, "RESULT of content compare: %ld\n", bp2->size);
          return (bp1->size - bp2->size);
      }
  }
}


/*
* Hash function for hashing the content of a blob.
*
* @param bp  The blob.
* @return  Hash of the blob.
*/
int blob_hash(BLOB *bp) {
  char* str_hash = bp->content;
  int hash = 0;

  while (*str_hash) {
      hash += *str_hash;
      str_hash++;
  }
  hash /= 2;
  hash += 13;

  return hash;
}


/*
* Create a key from a blob.
* The key inherits the caller's reference to the blob.
*
* @param bp  The blob.
* @return  the newly created key.
*/
KEY *key_create(BLOB *bp) {
  KEY* new_key = (KEY*)Malloc(sizeof(KEY));

  int hash = blob_hash(bp);
   new_key->hash = hash;
  new_key->blob = bp;

  // fprintf(stderr, "MAKE KEY{ Hash number %d, ", new_key->hash);
  // fprintf(stderr, "Blob: %s}\n", new_key->blob->prefix);

  return new_key;
}


/*
* Dispose of a key, decreasing the reference count of the contained blob.
* A key must be disposed of only once and must not be referred to again
* after it has been disposed.
*
* @param kp  The key.
*/
void key_dispose(KEY *kp) {
  blob_unref(kp->blob, "Disposing the KEY\n");
  Free(kp);
}


/*
* Compare two keys for equality.
*
* @param kp1  The first key.
* @param kp2  The second key.
* @return  0 if the keys are equal, otherwise nonzero.
*/
int key_compare(KEY *kp1, KEY *kp2) {
  if (kp1->hash == kp2->hash) {
      // fprintf(stderr, "SAME HASH COMPARE BLOBS--\n");

      int result = blob_compare(kp1->blob, kp2->blob);
      // fprintf(stderr, "result of KEY compare: %d\n", result);
      return result;
  } else {
      //  fprintf(stderr, "NOT THE SAME\n");
       if (kp1->hash > kp2->hash) {
          //  fprintf(stderr, "result of KEY compare: %d\n", (kp1->hash - kp2->hash));
           return (kp1->hash - kp2->hash);
       } else {
          //  fprintf(stderr, "result of KEY compare: %d\n", (kp1->hash - kp2->hash));
           return (kp1->hash - kp2->hash);
       }
  }
}


/*
* Create a version of a blob for a specified creator transaction.
* The version inherits the caller's reference to the blob.
* The reference count of the creator transaction is increased to
* account for the reference that is stored in the version.
*
* @param tp  The creator transaction.
* @param bp  The blob.
* @return  The newly created version.
*/
VERSION *version_create(TRANSACTION *tp, BLOB *bp) {
   VERSION* new_version = (VERSION*) Malloc(sizeof(VERSION));

   new_version->blob = bp;
   new_version->creator = tp;
   new_version->next = NULL;
   new_version->prev = NULL;

   trans_ref(tp, "version created calling transaction\n");

//    fprintf(stderr, "Created new version of: %s\n", bp->prefix);
//    fprintf(stderr, "refcount of %s: %d\n", bp->prefix, bp->refcnt);

   return new_version;
}


/*
* Dispose of a version, decreasing the reference count of the
* creator transaction and contained blob.  A version must be
* disposed of only once and must not be referred to again once
* it has been disposed.
*
* @param vp  The version to be disposed.
*/
void version_dispose(VERSION *vp) {
  // fprintf(stderr, "dispose  version of %s\n", vp->blob->prefix);

  blob_unref(vp->blob, "version disposed blob ref\n");
  trans_unref(vp->creator, "verion dispose trans ref\n");

  Free(vp);
}
