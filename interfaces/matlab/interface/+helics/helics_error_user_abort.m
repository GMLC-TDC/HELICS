function v = helics_error_user_abort()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 61);
  end
  v = vInitialized;
end
