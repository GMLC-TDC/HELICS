function v = other_error_type()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230864);
  end
  v = vInitialized;
end
