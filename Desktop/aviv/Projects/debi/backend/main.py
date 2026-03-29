from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.api.questions import router as questions_router
from app.api.profiles import router as profiles_router

app = FastAPI(
    title="Questions & Profiles API",
    description="Backend API for interacting with questions and user profiles",
    version="1.0.0"
)

# Configure CORS to allow cross-origin requests from the frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Update this in production with specific origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Include routers
app.include_router(questions_router, prefix="/api/questions", tags=["Questions"])
app.include_router(profiles_router, prefix="/api/profiles", tags=["Profiles"])

@app.get("/")
def read_root():
    return {"message": "Welcome to the API. Visit /docs for the interactive API documentation."}

if __name__ == "__main__":
    import uvicorn
    # Run server locally for development
    uvicorn.run("main:app", host="0.0.0.0", port=8000, reload=True)
