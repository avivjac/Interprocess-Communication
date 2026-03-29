from fastapi import APIRouter
from app.models.schemas import ProfileStatsResponse
from app.services.profile_service import get_user_profile_stats

router = APIRouter()

@router.get("/{user_id}/stats", response_model=ProfileStatsResponse)
def get_stats(user_id: str):
    """
    Returns the calculated profile statistics for a given user.
    """
    return get_user_profile_stats(user_id)
