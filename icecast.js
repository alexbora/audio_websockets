/**
 * @class       : icecast
 * @author      : alex (alex@mac.local)
 * @created     : Marţi Noi 07, 2023 09:14:29 EET
 * @description : icecast
 */

// import IcecastMetadataPlayer from 'icecast-metadata-player';
import {IcecastMetadataPlayer} from 'icecast-metadata-player';

const player = new IcecastMetadataPlayer('https://dsmrad.io/stream/isics-all', {
  onMetadata: (metadata) => {
    console.log(metadata)
  }
});

