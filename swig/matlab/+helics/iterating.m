function v = iterating()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176319);
  end
  v = vInitialized;
end
