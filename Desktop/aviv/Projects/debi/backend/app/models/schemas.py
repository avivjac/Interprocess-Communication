from pydantic import BaseModel
from typing import Optional, Dict, Any

class QuestionResponse(BaseModel):
    question_id: str
    text: str
    metadata: Optional[Dict[str, Any]] = None

class ProfileStatsResponse(BaseModel):
    user_id: str
    total_questions_answered: int
    score: float
    preferences: Optional[Dict[str, Any]] = None
