#pragma once

// Shop-style vehicle preview. The in-game vehicle websites (Legendary Motorsport
// = lgm_*, Southern SA Super Autos = sssa_*, Elitas = elt_*, LS Customs = lsc_*,
// Warstock = candc_*, Docktease = dock_*, Pegasus = pandm_*, ...) each ship a
// streamed texture dict holding one image per vehicle, named after the model.
// DRAW_SPRITE with the wrong dict draws the white "missing texture" fallback, so
// the right dict must be known: a background scan reads every website dict's
// texture-name-hash list out of its pgDictionary (via g_TxdStore) and builds a
// model-hash -> dict map. Ported from the Xbox 360 Insulin menu; the PS4 anchors
// (pgDictionary layout, g_TxdStore 0x3E4B218) come from the RE catalog.
namespace menu { namespace vehicle_preview {

    // Call every frame the class list is open, with the highlighted model
    // (nullptr = none). Starts and advances the background scan, pins the model's
    // dict and draws its image once it has safely finished streaming.
    void browse(const char* model);

    // Diagnostic self-test, one pass per boot. Once the scan has finished, runs
    // every model of a list through the same resolver browse() uses and logs the
    // ones with no showroom image, plus a count of which naming rule matched the
    // rest. Answers "which cars have no picture" without a human highlighting
    // hundreds of them by hand, and makes a rule that never fires visible.
    // Pure lookups: draws nothing, disturbs no selection, chunked across frames.
    // The list is passed as an accessor so this file needs no vehicle list.
    typedef const char* (*model_at_fn)(int index);
    void audit(model_at_fn at, int count);

    // Call every menu frame, always (from the global menu tick). Advances the
    // ready-frames settle gate and the deferred dict-release queue, and releases
    // the pinned dict a few frames after browse() stops (i.e. after the user
    // leaves the list) so a release never frees a texture still queued for draw.
    void tick();
}}
