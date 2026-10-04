{
    "patcher": {
        "fileversion": 1,
        "appversion": {
            "major": 9,
            "minor": 1,
            "revision": 5,
            "architecture": "x64",
            "modernui": 1
        },
        "classnamespace": "box",
        "rect": [ 1188.0, 341.0, 491.0, 446.0 ],
        "boxes": [
            {
                "box": {
                    "id": "obj-7",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 26.0, 22.0, 52.0, 20.0 ],
                    "presentation_linecount": 2,
                    "text": "physical"
                }
            },
            {
                "box": {
                    "id": "obj-6",
                    "maxclass": "comment",
                    "numinlets": 1,
                    "numoutlets": 0,
                    "patching_rect": [ 88.0, 22.0, 40.0, 20.0 ],
                    "text": "digital"
                }
            },
            {
                "box": {
                    "id": "obj-19",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 374.0, 279.0, 20.0, 140.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-18",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 374.0, 236.0, 85.0, 22.0 ],
                    "saved_object_attributes": {
                        "filename": "pickupmode",
                        "parameter_enable": 0
                    },
                    "text": "js pickupmode"
                }
            },
            {
                "box": {
                    "id": "obj-11",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 225.0, 236.0, 79.0, 22.0 ],
                    "saved_object_attributes": {
                        "filename": "scalemode",
                        "parameter_enable": 0
                    },
                    "text": "js scalemode"
                }
            },
            {
                "box": {
                    "id": "obj-12",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 225.0, 279.0, 20.0, 140.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-4",
                    "maxclass": "newobj",
                    "numinlets": 2,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 41.0, 236.0, 76.0, 22.0 ],
                    "saved_object_attributes": {
                        "filename": "glidemode",
                        "parameter_enable": 0
                    },
                    "text": "js glidemode"
                }
            },
            {
                "box": {
                    "id": "obj-3",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 41.0, 279.0, 20.0, 140.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-2",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 98.0, 56.0, 20.0, 140.0 ]
                }
            },
            {
                "box": {
                    "id": "obj-1",
                    "maxclass": "slider",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 41.0, 56.0, 20.0, 140.0 ]
                }
            }
        ],
        "lines": [
            {
                "patchline": {
                    "destination": [ "obj-11", 0 ],
                    "midpoints": [ 50.5, 222.0, 234.5, 222.0 ],
                    "order": 1,
                    "source": [ "obj-1", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-18", 0 ],
                    "midpoints": [ 50.5, 222.0, 383.5, 222.0 ],
                    "order": 0,
                    "source": [ "obj-1", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-4", 0 ],
                    "midpoints": [ 50.5, 198.0, 50.5, 198.0 ],
                    "order": 2,
                    "source": [ "obj-1", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-12", 0 ],
                    "midpoints": [ 234.5, 261.0, 234.5, 261.0 ],
                    "source": [ "obj-11", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-19", 0 ],
                    "midpoints": [ 383.5, 261.0, 383.5, 261.0 ],
                    "source": [ "obj-18", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-11", 1 ],
                    "midpoints": [ 107.5, 222.0, 294.5, 222.0 ],
                    "order": 1,
                    "source": [ "obj-2", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-18", 1 ],
                    "midpoints": [ 107.5, 222.0, 449.5, 222.0 ],
                    "order": 0,
                    "source": [ "obj-2", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-4", 1 ],
                    "midpoints": [ 107.5, 198.0, 107.5, 198.0 ],
                    "order": 2,
                    "source": [ "obj-2", 0 ]
                }
            },
            {
                "patchline": {
                    "destination": [ "obj-3", 0 ],
                    "midpoints": [ 50.5, 261.0, 50.5, 261.0 ],
                    "source": [ "obj-4", 0 ]
                }
            }
        ],
        "autosave": 0
    }
}