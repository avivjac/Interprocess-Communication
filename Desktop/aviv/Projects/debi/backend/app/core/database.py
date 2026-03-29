from supabase import create_client, Client
from app.core.config import settings

def get_supabase_client() -> Client | None:
    """
    Initializes and returns the Supabase client.
    Returns None if the credentials are not configured.
    """
    if not settings.SUPABASE_URL or not settings.SUPABASE_KEY:
        print("Warning: SUPABASE_URL or SUPABASE_KEY is missing. Database connection will be unavailable.")
        return None
    try:
        return create_client(settings.SUPABASE_URL, settings.SUPABASE_KEY)
    except Exception as e:
        print(f"Error initializing Supabase client: {e}")
        return None

# Singleton-like instance to be imported across the app
supabase = get_supabase_client()
